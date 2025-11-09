// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponInstance.h"
#include "../../Player/MyTournamentCharacter.h"
#include "../../Actors/Items/Projectile.h"
#include "../../Interfaces/Damageable.h"
#include "../../Actors/Components/InventoryComponent.h"
#include "../../Actors/Components/EntityVitalsComponent.h"
#include <Kismet/KismetMathLibrary.h>

// Sets default values
AWeaponInstance::AWeaponInstance()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_skeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	check(_skeletalMesh != nullptr);

	_additionalSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AdditionalSkeletalMesh"));
	check(_additionalSkeletalMesh != nullptr);

	_additionalSkeletalMesh->SetActive(false);
	_additionalSkeletalMesh->SetVisibility(false);
}

// Player-Only, binds the IA to AWeaponInstance functions
void AWeaponInstance::BindFirePrimaryAction(const UInputAction* InputToBind)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Attempting to bind AWeaponInstance FirePrimary to Player"));

	// Set up action bindings
	if (AMyTournamentCharacter* pc = Cast<AMyTournamentCharacter>(_weaponOwner))
	{
		if (UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(pc->InputComponent))
		{
			// Fire
			enhancedInputComponent->BindAction(InputToBind, ETriggerEvent::Triggered, this, &AWeaponInstance::FirePrimary);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Done!"));
		}
	}
}


// Called when the game starts or when spawned
void AWeaponInstance::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponInstance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Sets both the weapon owner and the ammoValue ptr 
void AWeaponInstance::SetWeaponOwner(AActor* ownerToSet)
{
	if (ownerToSet)
	{
		_weaponOwner = ownerToSet;

		// Check if owner has the WeaponOperator interface, otherwise that owner shouldn't really own this weapon
		if (!_weaponOwner->GetClass()->ImplementsInterface(UWeaponOperator::StaticClass()))
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("_weaponOwner does not implement IWeaponOperator!"));
	
		// Check if owner has inventory and set reference
		if(!(_ownersInventory = ownerToSet->FindComponentByClass<UInventoryComponent>()))
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("_weaponOwner does not have an Inventory!"));
	}
}

AActor* AWeaponInstance::GetWeaponOwner()
{
	return _weaponOwner;
}

void AWeaponInstance::FirePrimary()
{
	if(_ownersInventory->GetAmmoCount(_weaponAsset->_ammoType) > 0)
		HandleFirePrimary();
}

void AWeaponInstance::HandleFirePrimary()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Firing the weapon!"));

	// Make sure weaponOwner is valid and implements IWeaponOperator
	if (IsValid(_weaponOwner) && _weaponOwner->GetClass()->ImplementsInterface(UWeaponOperator::StaticClass()))
	{
		if (_weaponAsset->_shootingType == EShootingType::Bullet)
		{
			FVector targetPosition = IWeaponOperator::Execute_GetAimPoint(_weaponOwner);

			// Get the correct socket to spawn the projectile from
			FVector socketLocation = _skeletalMesh->GetSocketLocation("Muzzle");
			FRotator spawnRotation = UKismetMathLibrary::FindLookAtRotation(socketLocation, targetPosition);
			FVector spawnLocation = socketLocation + UKismetMathLibrary::GetForwardVector(spawnRotation) * 10.0;

			SpawnBullet(spawnLocation, spawnRotation);
		}
		else if (_weaponAsset->_shootingType == EShootingType::Raycast)
		{
			// RaycastBullet
			FVector targetPosition = IWeaponOperator::Execute_GetAimPoint(_weaponOwner);
			FVector socketLocation = _skeletalMesh->GetSocketLocation("Muzzle");
			FRotator spawnRotation = UKismetMathLibrary::FindLookAtRotation(socketLocation, targetPosition);
			FVector start = socketLocation + UKismetMathLibrary::GetForwardVector(spawnRotation) * 10.0;

			RaycastBullet(start, targetPosition);
		}

		_ownersInventory->ConsumeAmmo(_weaponAsset->_ammoType, 1);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("_weaponOwner is not valid or does not implement IWeaponOperator!"));
	}
}

void AWeaponInstance::SpawnBullet(FVector spawnLocation, FRotator spawnRotation)
{
	UWorld* const World = GetWorld();

	if (World && _projectileClass != nullptr)
	{
		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// Spawn the projectile at the muzzle
		AProjectile* prj = World->SpawnActor<AProjectile>(_projectileClass, spawnLocation, spawnRotation, ActorSpawnParams);

		if (prj)
			prj->InitializeProjectile(_weaponOwner, this);
	}
}

void AWeaponInstance::RaycastBullet(FVector start, FVector end)
{
	UWorld* const World = GetWorld();

	// Trace the wall, so we can stick to it
	FHitResult hitRes;

	const ECollisionChannel WeaponRaycast = ECollisionChannel::ECC_GameTraceChannel2; // safer than hardcoding
	FCollisionQueryParams collParams(SCENE_QUERY_STAT(WeaponRaycast), false);
	collParams.AddIgnoredActor(this);
	collParams.AddIgnoredActor(_weaponOwner);

	// Debug the trace
	if(_debugRaycastBullet)
		DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 3.0f, 0, 1.0f);

	// Check if we're still attached to the wall (if the player looks away, this will fail, so the 'else' will correctly deatach him from the wall)
	if (GetWorld()->LineTraceSingleByChannel(hitRes, start, end, WeaponRaycast, collParams))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Hit %s"), *hitRes.GetActor()->GetName()));

		AActor* otherActor = hitRes.GetActor();
		UPrimitiveComponent* otherComponent = hitRes.GetComponent();

		// Check if the actor hit has vitals
		if (auto* comp = otherActor->FindComponentByClass<UEntityVitalsComponent>())
		{
			float dmg = _weaponAsset->_damage;
			IDamageable::Execute_ApplyDamage(comp, dmg);
		}

		if (otherComponent->IsSimulatingPhysics())
		{
			const FVector shotDir = (end - start).GetSafeNormal();
			const FVector impulse = shotDir * _weaponAsset->_impulseForceOnHit;

			otherComponent->AddImpulseAtLocation(impulse, hitRes.ImpactPoint);
		}
	}
}
