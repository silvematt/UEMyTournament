// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
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
	//_collisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnOverlap);
	_collisionComponent->OnComponentHit.AddUniqueDynamic(this, &AProjectile::OnHit);

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

	// Ignore the actor that shot this bullet
	_collisionComponent->IgnoreActorWhenMoving(actorThatShot, true);

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

	// Only add impulse and destroy projectile if we hit a physics object
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor != _actorThatShot) && (OtherComp != nullptr))
	{
		if (_isAnExplosiveBullet)
		{
			// Perform explosion
			ExplosionCheck(GetActorLocation());
		}
		else // Normal bullet
		{
			// Check if the actor hit has vitals
			if (auto* comp = OtherActor->FindComponentByClass<UEntityVitalsComponent>())
			{
				float dmg = _weaponThatShot->_weaponAsset->_damage;
				float locationalMultiplier = IDamageable::Execute_GetLocationalDamageMultiplier(comp, Hit.BoneName);
				float finalDmg = dmg * locationalMultiplier;

				IDamageable::Execute_ApplyDamage(comp, finalDmg);
			}
		}

		if (OtherComp->IsSimulatingPhysics())
			OtherComp->AddImpulseAtLocation(GetVelocity().GetSafeNormal() * _weaponThatShot->_weaponAsset->_impulseForceOnHit, GetActorLocation());

		// Spawn VFX if set
		if (_impactVFX)
		{
			const FTransform SpawnTM(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint);

			GetWorld()->SpawnActor<AActor>(_impactVFX, SpawnTM);
		}

		Destroy();
	}
}

void AProjectile::ExplosionCheck(const FVector& center)
{
	// Sphere overlap check look for nearby actors to damage
	TArray<FOverlapResult> overlaps;

	FCollisionShape overlapSphere;
	overlapSphere.SetSphere(_explosionRadius);

	FCollisionObjectQueryParams objectParams;
	objectParams.AddObjectTypesToQuery(ECC_Pawn);
	objectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	objectParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionQueryParams queryParams;

	GetWorld()->OverlapMultiByObjectType(overlaps, center, FQuat::Identity, objectParams, overlapSphere, queryParams);

	TArray<AActor*> damagedActors;

	// Process the overlap results
	for (const FOverlapResult& CurrentOverlap : overlaps)
	{
		// Overlaps may return the same actor multiple times per each component overlapped
		// ensure we only damage each actor once by adding it to a damaged list
		if (damagedActors.Find(CurrentOverlap.GetActor()) == INDEX_NONE)
		{
			damagedActors.Add(CurrentOverlap.GetActor());

			// Apply physics force away from the explosion
			const FVector& explosionDirection = CurrentOverlap.GetActor()->GetActorLocation() - GetActorLocation();

			// Check if the actor hit has vitals
			if (auto* comp = CurrentOverlap.GetActor()->FindComponentByClass<UEntityVitalsComponent>())
			{
				float dmg = _weaponThatShot->_weaponAsset->_damage;
				IDamageable::Execute_ApplyDamage(comp, dmg);

				if(_launchCharactersOnExplosionHit)
					if (ACharacter* hitCharacter = Cast<ACharacter>(CurrentOverlap.GetActor()))
					{
						// Tune these values to taste
						FVector launchVelocity = explosionDirection.GetSafeNormal() * _explosionLaunchHorizontalForce;
						launchVelocity.Z += _explosionLaunchVerticalForce;

						hitCharacter->LaunchCharacter(launchVelocity, true, true);
					}
			}

			if (CurrentOverlap.GetComponent()->IsSimulatingPhysics())
				CurrentOverlap.GetComponent()->AddImpulseAtLocation(explosionDirection * _explosionForce, GetActorLocation());
		}
	}
}
