// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponInstance.generated.h"

class UWeaponAsset;
class UInventoryComponent;

UCLASS()
class MYTOURNAMENT_API AWeaponInstance : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponInstance();

// Properties
protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UWeaponAsset> _weaponAsset;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> _ownersInventory;

	UPROPERTY()
	TObjectPtr<AActor> _weaponOwner;
	

public:
	// Components
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> _skeletalMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> _additionalSkeletalMesh; // used for the TP weapon for the player, mesh is set at runtime

	// Data, anim blueprints to play when this weapon is used
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimBlueprint> _fpsAnimBlueprint;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimBlueprint> _tpsAnimBlueprint;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	AActor* GetWeaponOwner();

	UFUNCTION()
	void SetWeaponOwner(AActor* ownerToSet);
};
