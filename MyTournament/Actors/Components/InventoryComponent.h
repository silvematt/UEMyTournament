// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Data/Items/WeaponAsset.h"
#include "InventoryComponent.generated.h"

class AWeaponInstance;

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


// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponIsAddedSignature, UWeaponAsset*, weaponAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponIsEquippedSignature, const FWeaponInInventoryEntry&, weaponEntryEquipped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponIsUnequippedSignature, const FWeaponInInventoryEntry&, weaponEntryUnequipped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoIsAddedSignature, UAmmoType*, ammoType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoIsConsumedSignature, UAmmoType*, ammoType, uint32, newAmmoVal);


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

	UPROPERTY(EditAnywhere)
	uint32 _defaultWeaponAmmoCount = 60;

	UPROPERTY(VisibleAnywhere)
	EWeaponSlot _currentWeaponSlot = EWeaponSlot::Slot0; // slot0 means not-initialized

	UPROPERTY(EditAnywhere)
	TMap<EWeaponSlot, FWeaponInInventoryEntry> _weapons;

	UPROPERTY(VisibleAnywhere)
	TMap<TObjectPtr<UAmmoType>, uint32> _ammo;

// Delegates
public:
	FOnWeaponIsAddedSignature _onWeaponIsAddedDelegate;
	FOnWeaponIsEquippedSignature _onWeaponIsEquippedDelegate;
	FOnWeaponIsUnequippedSignature _onWeaponIsUnequippedDelegate;
	FOnAmmoIsAddedSignature _onAmmoIsAddedDelegate;
	FOnAmmoIsConsumedSignature _onAmmoIsConsumedDelegate;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// CustomInitialize is called by the owner of this component (like the player)
	UFUNCTION()
	void CustomInitialize();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Tries to add the weapon, this can fail if weapon is already in this inventory and ammo is maxed out
	UFUNCTION()
	bool TryAddWeapon(UWeaponAsset* weaponToAdd, uint32 ammoCount);

	UFUNCTION()
	bool TryAddAmmo(UAmmoType* ammoToAdd, uint32 ammoCount);

	// Tries to spawn/enable the weapon instance
	UFUNCTION()
	bool TryEquip(EWeaponSlot slot);

	UFUNCTION()
	bool UnequipCurrentWeapon();

	// Returns the ammo of the currently equipped weapon (if any)
	UFUNCTION()
	int32 GetCurrentWeaponAmmoCount() const;

	// Consumes 'val' of the passed ammo type
	UFUNCTION()
	void ConsumeAmmo(UAmmoType* ammo, uint32 val);

	// Returns the ammo count of a specific type
	UFUNCTION()
	uint32 GetAmmoCount(UAmmoType* ammo);
};
