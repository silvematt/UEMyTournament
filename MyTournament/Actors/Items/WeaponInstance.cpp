// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponInstance.h"
#include "../../Player/MyTournamentCharacter.h"
#include "../../Actors/Items/Projectile.h"
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

void AWeaponInstance::SetWeaponOwner(AActor* ownerToSet)
{
	if (ownerToSet)
	{
		_weaponOwner = ownerToSet;

		if (!_weaponOwner->GetClass()->ImplementsInterface(UWeaponOperator::StaticClass()))
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("_weaponOwner does not implement IWeaponOperator!"));
	}
}

AActor* AWeaponInstance::GetWeaponOwner()
{
	return _weaponOwner;
}

void AWeaponInstance::FirePrimary()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Firing the weapon!"));

	HandleFirePrimary();
}

void AWeaponInstance::HandleFirePrimary()
{
	UWorld* const World = GetWorld();

	if (World != nullptr && _projectileClass != nullptr)
	{
		// Make sure weaponOwner is valid and implements IWeaponOperator
		if (IsValid(_weaponOwner) && _weaponOwner->GetClass()->ImplementsInterface(UWeaponOperator::StaticClass()))
		{
			FVector targetPosition = IWeaponOperator::Execute_GetAimPoint(_weaponOwner);

			// Get the correct socket to spawn the projectile from
			FVector socketLocation = _skeletalMesh->GetSocketLocation("Muzzle");
			FRotator spawnRotation = UKismetMathLibrary::FindLookAtRotation(socketLocation, targetPosition);
			FVector spawnLocation = socketLocation + UKismetMathLibrary::GetForwardVector(spawnRotation) * 10.0;

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			
			// Spawn the projectile at the muzzle
			AProjectile* prj = World->SpawnActor<AProjectile>(_projectileClass, spawnLocation, spawnRotation, ActorSpawnParams);

			if(prj)
				prj->InitializeProjectile(_weaponOwner, this);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("_weaponOwner is not valid or does not implement IWeaponOperator!"));
		}
	}
}
