// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../Interfaces/WeaponOperator.h"
#include "WeaponInstance.generated.h"

class UWeaponAsset;
class UInventoryComponent;
class UInputMappingContext;
class UInputAction;
class AProjectile;
class UNiagaraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFiresSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFiresSecondarySignature);


UCLASS()
class MYTOURNAMENT_API AWeaponInstance : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponInstance();

// Properties
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AActor> _weaponOwner;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> _ownersInventory;

	// Array containing the input handles bound during instantiating this weapon (player only)
	TArray<int32> _inputBoundHandles;

	UPROPERTY()
	bool _isWeaponActivated = false; // becomes true when the weapon is drawn on switch

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UWeaponAsset> _weaponAsset;

	// Components
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> _skeletalMesh; // Main weapon mesh

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> _additionalSkeletalMesh; // Additional mesh used for the TP weapon for the player, the actual SK_Mesh is set at runtime the same as _skeletalMesh

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraComponent> _muzzleFX;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<UAudioComponent> _audioComponent;

	// True if the IA_Fire action is being held down (can be set false via code, like if the player is firing an automatic weapon and he runs out of ammo)
	UPROPERTY(VisibleAnywhere)
	bool _bIsTriggerHeld = false;

	UPROPERTY(VisibleAnywhere)
	bool _bIsSecondTriggerHeld = false;

	// Allows to shoot at the set fire rate
	UPROPERTY(VisibleAnywhere)
	float _fireTimer = 0.0f;

	// Burst fire mode management
	UPROPERTY(VisibleAnywhere)
	bool _burstStarted = false;

	// How many bullets were shot in the current burst
	UPROPERTY(VisibleAnywhere)
	int _burstNumShot = 0;

	UPROPERTY(VisibleAnywhere)
	float _spreadProgressiveAccumulation = 0.0f;

	// Debugs the raycast shot if this weapon doesn't instantiate bullets
	UPROPERTY(EditAnywhere)
	bool _debugRaycastBullet = false;

public:
	// Delegates
	FOnWeaponFiresSignature _onWeaponFiresPrimaryDelegate;
	FOnWeaponFiresSecondarySignature _onWeaponFiresSecondaryDelegate;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Spawns a Bullet when FireOneShot determines so
	UFUNCTION()
	void SpawnBullet(FVector spawnLocation, FRotator spawnRotation);

	// Fires a LineTrace when FireOneShot determines so
	UFUNCTION()
	void RaycastBullet(FVector start, FVector end);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Player-Only, binds the IA to AWeaponInstance functions
	UFUNCTION()
	void BindFirePrimaryAction(const UInputAction* InputToBind);

	UFUNCTION()
	void BindFireSecondaryAction(const UInputAction* InputToBind);

	// Player-Only, unbinds the IA on weapons deactivation/destroy
	UFUNCTION()
	void UnbindInputActions();

	// Must be done as soon as this weapon instance is spawned
	UFUNCTION()
	void SetWeaponOwner(AActor* ownerToSet);

	UFUNCTION()
	AActor* GetWeaponOwner();

	// FirePrimary input action
	UFUNCTION()
	void FirePrimary();

	// Called on input release or if the weapons has to stop firing
	UFUNCTION()
	void StopFiringPrimary();

	// FireSecondary input action
	UFUNCTION()
	void FireSecondary();

	// Called on input release or if the weapons has to stop firing
	UFUNCTION()
	void StopFiringSecondary();

	// Fires one shot at the _weaponAssets conditions
	UFUNCTION()
	void FireOneShot();

	// Firing Handling called in Tick()
	UFUNCTION()
	void HandleFiring();

	UFUNCTION()
	void SetWeaponActivated(bool v) { _isWeaponActivated = v; }
};
