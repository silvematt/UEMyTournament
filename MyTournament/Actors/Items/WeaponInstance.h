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

UCLASS()
class MYTOURNAMENT_API AWeaponInstance : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponInstance();

// Properties
protected:
	UPROPERTY()
	TObjectPtr<AActor> _weaponOwner;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> _ownersInventory;

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UWeaponAsset> _weaponAsset;

	// Components
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> _skeletalMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> _additionalSkeletalMesh; // used for the TP weapon for the player, mesh is set at runtime

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputMappingContext> _weaponMappingContext; // this is set to the player when the weapon is equipped

	// Data, anim blueprints to play when this weapon is used
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimBlueprint> _fpsAnimBlueprint;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimBlueprint> _tpsAnimBlueprint;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AProjectile> _projectileClass;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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

	UFUNCTION()
	void FirePrimary();

	UFUNCTION()
	void HandleFirePrimary();
};
