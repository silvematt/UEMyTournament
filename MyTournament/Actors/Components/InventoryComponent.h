// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Data/Items/WeaponAsset.h"
#include "InventoryComponent.generated.h"

class AWeaponInstance;
class UInputAction;

/*
	This struct represents a wepapon present in the inventory
	_asset is always going to be set
	_instance only when the weapon is active or waiting to be gc collected (TODO we can cache the instance instead of destroying it and instantiating it every time the weapon is swapped)
*/
USTRUCT(BlueprintType)
struct FWeaponInInventoryEntry
{
	GENERATED_BODY()

	FWeaponInInventoryEntry() = default;

	FWeaponInInventoryEntry(UWeaponAsset* InAsset) : _asset(InAsset) {}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UWeaponAsset> _asset;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
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

	UPROPERTY()
	TObjectPtr<AActor> _inventoryOwner;

	// Weapon the owner of this inventory has as soon as he spawns. This could have been an array if we wanted to have multiple weapons upon spawn
	UPROPERTY(EditAnywhere)
	TObjectPtr<UWeaponAsset> _defaultWeapon;

	UPROPERTY(EditAnywhere)
	uint32 _defaultWeaponAmmoCount = 60;

	UPROPERTY(VisibleAnywhere)
	EWeaponSlot _currentWeaponSlot = EWeaponSlot::None; // None means not-initialized

	UPROPERTY(VisibleAnywhere)
	TMap<EWeaponSlot, FWeaponInInventoryEntry> _weapons;

	UPROPERTY(EditAnywhere) // it's probably useful to be able to set the ammo value in the details panel
	TMap<TObjectPtr<UAmmoType>, uint32> _ammo;

	// Input Actions
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> _IAWeaponSlotOne;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> _IAWeaponSlotTwo;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> _IAWeaponSlotThree;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> _IAWeaponSlotFour;

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
	void CustomInitialize(AActor* invOwner);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Player-Only, binds the IActions to AWeaponInstance functions
	UFUNCTION()
	void BindWeaponSwitchActions();

	// Tries to add the weapon, this can fail if weapon is already in this inventory and ammo is maxed out
	UFUNCTION()
	bool TryAddWeapon(UWeaponAsset* weaponToAdd, uint32 ammoCount);

	UFUNCTION()
	bool TryAddAmmo(UAmmoType* ammoToAdd, uint32 ammoCount);

	// Tries to spawn/enable the weapon instance of _weapons[slot]
	UFUNCTION()
	bool TryEquip(EWeaponSlot slot);

	UFUNCTION()
	bool UnequipCurrentWeapon();

	// Returns the ammo of the currently equipped weapon (if any)
	UFUNCTION()
	uint32 GetCurrentWeaponAmmoCount() const;

	// Consumes 'val' of the passed ammo type
	UFUNCTION()
	void ConsumeAmmo(UAmmoType* ammo, uint32 val);

	// Returns the ammo count of a specific type
	UFUNCTION()
	uint32 GetAmmoCount(UAmmoType* ammo) const;

	UFUNCTION()
	void SwitchWeaponInputAction(const FInputActionValue& Value, const EWeaponSlot slot);

	UFUNCTION()
	bool SwitchWeapon(EWeaponSlot slot);

	UFUNCTION()
	UWeaponAsset* GetCurrentWeaponAsset();

	UFUNCTION()
	AWeaponInstance* GetCurrentWeaponInstance();

};
