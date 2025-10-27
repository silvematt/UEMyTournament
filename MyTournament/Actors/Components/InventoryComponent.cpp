// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Intialize
	if (_defaultWeapon)
	{
		TryAddWeapon(_defaultWeapon);
	}
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UInventoryComponent::TryAddWeapon(UWeaponAsset* weaponToAdd)
{
	if (!weaponToAdd) 
		return false;

	// Check if the weapon isn't already added
	if (!_weapons.Contains(weaponToAdd->_weaponSlot))
	{
		_weapons.Add(weaponToAdd->_weaponSlot, FWeaponInInventoryEntry(weaponToAdd));

		// If no weapon was equipped, equip this one
		if (_currentWeaponSlot == EWeaponSlot::Slot0)
			TryEquip(weaponToAdd->_weaponSlot);
		// Otherwise if the currently equipped weapon has no ammo, equip the new one
		else if (GetCurrentWeaponAmmoCount() == 0)
			TryEquip(weaponToAdd->_weaponSlot);

		return true;
	}
	else
		return false;
}

bool UInventoryComponent::TryEquip(EWeaponSlot slot)
{

	return false;
}

int32 UInventoryComponent::GetCurrentWeaponAmmoCount() const
{
	if (_currentWeaponSlot == EWeaponSlot::Slot0)
		return 0;

	const FWeaponInInventoryEntry* entry = _weapons.Find(_currentWeaponSlot);
	if (!entry)
		return 0;

	const UAmmoType* ammoType = entry->_asset->_ammoType;
	const int32 ammoCount = _ammo.FindRef(ammoType); // returns 0 if not found
	return ammoCount;
}
