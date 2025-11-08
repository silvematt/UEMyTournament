// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "../../Data/Items/WeaponAsset.h"
#include "../../Actors/Items/WeaponInstance.h"
#include "../Components/EntityVitalsComponent.h"
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

	// Callback on projectile hits something
	_collisionComponent->SetGenerateOverlapEvents(true);
	_collisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnOverlap);

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

void AProjectile::InitializeProjectile(AActor* actorThatShot, AWeaponInstance* weaponThatShot)
{
	_actorThatShot = actorThatShot;
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

void AProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!_shot)
		return;

	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor != _actorThatShot) && (OtherComp != nullptr))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Hit %s"), *OtherActor->GetName()));

		// Check if the actor hit has vitals
		if (auto* comp = OtherActor->FindComponentByClass<UEntityVitalsComponent>())
		{
			float dmg = _weaponThatShot->_weaponAsset->_damage;
			IDamageable::Execute_ApplyDamage(comp, dmg);
		}

		if(OtherComp->IsSimulatingPhysics())
			OtherComp->AddImpulseAtLocation(GetVelocity() * _impulseForceOnHit, GetActorLocation());

		Destroy();
	}
}
