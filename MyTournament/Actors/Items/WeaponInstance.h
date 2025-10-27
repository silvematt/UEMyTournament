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
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWeaponAsset> _weaponAsset;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInventoryComponent> _ownersInventory;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
