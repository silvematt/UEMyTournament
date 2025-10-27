// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Data/Items/WeaponAsset.h"
#include "InventoryComponent.generated.h"

class AWeaponInstance;

// WeaponEntry can be useful to 
USTRUCT(BlueprintType)
struct FWeaponInInventoryEntry
{
	GENERATED_BODY()

	FWeaponInInventoryEntry() = default;

	FWeaponInInventoryEntry(UWeaponAsset* InAsset) : _asset(InAsset) {}

	UPROPERTY()
	TObjectPtr<UWeaponAsset> _asset;

	UPROPERTY()
	TObjectPtr<AWeaponInstance> _instance;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYTOURNAMENT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

// Properties
protected:
	// Weapon the owner of this inventory has as soon as he spawns
	UPROPERTY(EditAnywhere)
	TObjectPtr<UWeaponAsset> _defaultWeapon;

	UPROPERTY(VisibleAnywhere)
	EWeaponSlot _currentWeaponSlot = EWeaponSlot::Slot0; // slot0 means not-initialized

	UPROPERTY(EditAnywhere)
	TMap<EWeaponSlot, FWeaponInInventoryEntry> _weapons;

	UPROPERTY(VisibleAnywhere)
	TMap<TObjectPtr<UAmmoType>, int32> _ammo;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Tries to add the weapon, this can fail if weapon is already in this inventory and ammo is maxed out
	bool TryAddWeapon(UWeaponAsset* weaponToAdd);

	// Tries to spawn/enable the weapon instance
	bool TryEquip(EWeaponSlot slot);

	int32 GetCurrentWeaponAmmoCount() const;
};
