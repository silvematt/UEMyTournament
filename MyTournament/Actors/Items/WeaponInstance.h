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

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UWeaponAsset> _weaponAsset;

	// Components
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> _skeletalMesh; // Main weapon mesh

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> _additionalSkeletalMesh; // Additional mesh used for the TP weapon for the player, the actual SK_Mesh is set at runtime the same as _skeletalMesh

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputMappingContext> _weaponMappingContext; // this is set to the player when the weapon is equipped

	// Data, anim blueprints to play when this weapon is used
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimBlueprint> _fpsAnimBlueprint;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimBlueprint> _tpsAnimBlueprint;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AProjectile> _projectileClass;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraComponent> _muzzleFX;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<USoundBase> _fireSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<UAudioComponent> _audioComponent;

	// True if the IA_Fire action is being held down (can be set false via code, like if the player is firing an automatic weapon and he runs out of ammo)
	UPROPERTY(VisibleAnywhere)
	bool _bIsTriggerHeld = false;

	// Allows to shoot at the set fire rate
	UPROPERTY(VisibleAnywhere)
	float _fireTimer = 0.0f;;

	// Burst fire mode management
	UPROPERTY(VisibleAnywhere)
	bool _burstStarted = false;

	// How many bullets were shot in the current burst
	UPROPERTY(VisibleAnywhere)
	int _burstNumShot = 0;

	// Debugs the raycast shot if this weapon doesn't instantiate bullets
	UPROPERTY(EditAnywhere)
	bool _debugRaycastBullet = false;

public:
	// Delegates
	FOnWeaponFiresSignature _onWeaponFiresPrimary;


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
	void StopFiring();

	// Fires one shot at the _weaponAssets conditions
	UFUNCTION()
	void FireOneShot();

	// Firing Handling called in Tick()
	UFUNCTION()
	void HandleFiring();
};
