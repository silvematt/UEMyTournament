// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	_staticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	check(_staticMeshComponent != nullptr);

	_collisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	check(_collisionComponent != nullptr);

	_movementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	check(_movementComponent != nullptr);

	// Set collision component
	_collisionComponent->SetSphereRadius(5.0f);
	_collisionComponent->BodyInstance.SetCollisionProfileName("Projectile");

	// Callback on projectile hits something
	_collisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);

	// Set hierarchy
	RootComponent = _collisionComponent;
	_staticMeshComponent->SetupAttachment(_collisionComponent);

	// Initialize Projetile Movement
	_movementComponent->UpdatedComponent = _collisionComponent;
	_movementComponent->InitialSpeed = _travelSpeed;
	_movementComponent->MaxSpeed = _travelSpeed;
	_movementComponent->bRotationFollowsVelocity = true;
	_movementComponent->bShouldBounce = false;
	
	InitialLifeSpan = _lifespan;
}

void AProjectile::InitializeProjectile(AWeaponInstance* weaponThatShot)
{
	_weaponThatShot = weaponThatShot;
	_shot = true;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!_shot)
		return;

	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * _impulseForceOnHit, GetActorLocation());

		Destroy();
	}
}
