// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponInstance.h"
#include "../../Player/MyTournamentCharacter.h"
#include "../../Actors/Items/Projectile.h"
#include "../../Interfaces/Damageable.h"
#include "../../Actors/Components/InventoryComponent.h"
#include "../../Actors/Components/EntityVitalsComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include <Kismet/KismetMathLibrary.h>

// Sets default values
AWeaponInstance::AWeaponInstance()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_skeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	check(_skeletalMesh != nullptr);

	_muzzleFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MuzzleFx"));
	check(_muzzleFX != nullptr);
	_muzzleFX->SetupAttachment(_skeletalMesh, "Muzzle");
	_muzzleFX->bAutoActivate = false;

	_additionalSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AdditionalSkeletalMesh"));
	check(_additionalSkeletalMesh != nullptr);

	_audioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	check(_audioComponent != nullptr);
	_audioComponent->SetupAttachment(_skeletalMesh);

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
			auto& bind1 = enhancedInputComponent->BindAction(InputToBind, ETriggerEvent::Started, this, &AWeaponInstance::FirePrimary);
			auto& bind2 = enhancedInputComponent->BindAction(InputToBind, ETriggerEvent::Completed, this, &AWeaponInstance::StopFiring);

			_inputBoundHandles.Add(bind1.GetHandle());
			_inputBoundHandles.Add(bind2.GetHandle());

			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Done!"));
		}
	}
}

void AWeaponInstance::UnbindInputActions()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Attempting to unbind AWeaponInstance FirePrimary to Player"));

	// Remove previously made bindings
	if (AMyTournamentCharacter* pc = Cast<AMyTournamentCharacter>(_weaponOwner))
	{
		if (UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(pc->InputComponent))
		{
			for (int32 cur : _inputBoundHandles)
				enhancedInputComponent->RemoveBindingByHandle(cur);

			_inputBoundHandles.Reset();
			
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Done!"));
		}
	}
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
		if (!(_ownersInventory = ownerToSet->FindComponentByClass<UInventoryComponent>()))
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("_weaponOwner does not have an Inventory!"));
	}
}

AActor* AWeaponInstance::GetWeaponOwner()
{
	return _weaponOwner;
}


// Called when the game starts or when spawned
void AWeaponInstance::BeginPlay()
{
	Super::BeginPlay();

	_fireTimer = _weaponAsset->_fireRate;
	_audioComponent->SetSound(_fireSound);
}

// Called every frame
void AWeaponInstance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Advance the fireTimer, when _fireTimer >= _weaponAsset->fireRate, we can shoot one bullet
	_fireTimer += 1 * DeltaTime;

	HandleFiring();
}

void AWeaponInstance::HandleFiring()
{
	// Owner's current weapon is this one if this function is running

	if (_burstStarted)
	{
		// If no ammo, stop burst and return
		if (_ownersInventory->GetCurrentWeaponAmmoCount() <= 0)
		{
			_burstStarted = false;
			_burstNumShot = 0;
			StopFiring();
			// Dry fire
			return;
		}

		// Otherwise keep the burst going until FireOneShot disables it
		if (_fireTimer >= _weaponAsset->_burstFireRate)
		{
			FireOneShot();
			_fireTimer = 0.0f; // reset fire timer
		}
	}
	else if (_bIsTriggerHeld) 
	{
		if (_ownersInventory->GetCurrentWeaponAmmoCount() <= 0)
		{
			// Dry fire
			StopFiring();
			return;
		}
		if (_fireTimer >= _weaponAsset->_fireRate)
		{
			FireOneShot();
			_fireTimer = 0.0f; // reset fire timer
		}
	}
}

void AWeaponInstance::FirePrimary()
{
	// if(_fireTimer >= _weaponAsset->_fireRate) If the weapon can't fire, you may not want to pick up the input (you can double tap for continuous burst otherwise), but it can feel more clunky
	_bIsTriggerHeld = true;
}

void AWeaponInstance::StopFiring()
{
	_bIsTriggerHeld = false;
}

void AWeaponInstance::FireOneShot()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Firing the weapon!"));

	// Make sure weaponOwner is valid and implements IWeaponOperator
	if (IsValid(_weaponOwner) && _weaponOwner->GetClass()->ImplementsInterface(UWeaponOperator::StaticClass()))
	{
		// Physical Bullet
		if (_weaponAsset->_shootingType == EShootingType::Bullet)
		{
			FVector targetPosition = IWeaponOperator::Execute_GetAimPoint(_weaponOwner);

			// Get the correct socket to spawn the projectile from
			FVector socketLocation = _skeletalMesh->GetSocketLocation("Muzzle");
			FRotator spawnRotation = UKismetMathLibrary::FindLookAtRotation(socketLocation, targetPosition);
			FVector spawnLocation = socketLocation + UKismetMathLibrary::GetForwardVector(spawnRotation) * 10.0;

			SpawnBullet(spawnLocation, spawnRotation);
		}
		// RaycastBullet
		else if (_weaponAsset->_shootingType == EShootingType::Raycast)
		{
			FVector targetPosition = IWeaponOperator::Execute_GetAimPoint(_weaponOwner);
			FVector socketLocation = _skeletalMesh->GetSocketLocation("Muzzle");
			FRotator spawnRotation = UKismetMathLibrary::FindLookAtRotation(socketLocation, targetPosition);
			FVector start = socketLocation + UKismetMathLibrary::GetForwardVector(spawnRotation) * 10.0;

			RaycastBullet(start, targetPosition);
		}

		_ownersInventory->ConsumeAmmo(_weaponAsset->_ammoType, 1);
		_muzzleFX->Activate(true);
		_audioComponent->Play();
		_onWeaponFiresPrimary.Broadcast();

		// If this weapon is semi, release the trigger after we shot
		if(_weaponAsset->_fireMode == EFireMode::Single)
			StopFiring();
		// Burst Logic
		else if (_weaponAsset->_fireMode == EFireMode::Burst)
		{
			// Add a shot fired to the burst
			_burstStarted = true;
			_burstNumShot++;

			// Burst Completed
			if (_burstNumShot >= _weaponAsset->_burstShotsToFire)
			{
				_burstStarted = false;
				_burstNumShot = 0;
				StopFiring();
			}
		}
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

		// Add physics impulse
		if (otherComponent->IsSimulatingPhysics())
		{
			const FVector shotDir = (end - start).GetSafeNormal();
			const FVector impulse = shotDir * _weaponAsset->_impulseForceOnHit;

			otherComponent->AddImpulseAtLocation(impulse, hitRes.ImpactPoint);
		}
	}
}
