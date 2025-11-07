// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "../Items/WeaponInstance.h"

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

}

// CustomInitialize is called by the owner of this component (like the player)
void UInventoryComponent::CustomInitialize()
{
	// Intialize
	if (_defaultWeapon)
	{
		//TryAddWeapon(_defaultWeapon);
	}
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UInventoryComponent::TryAddWeapon(UWeaponAsset* weaponToAdd, bool addAmmo, int ammoCount)
{
	if (!weaponToAdd) 
		return false;

	// Check if the weapon was never added to the inventory, in case, add it
	if (!_weapons.Contains(weaponToAdd->_weaponSlot))
		_weapons.Add(weaponToAdd->_weaponSlot, FWeaponInInventoryEntry(weaponToAdd));

	// If no weapon was equipped, equip this one
	if (_currentWeaponSlot == EWeaponSlot::Slot0)
		TryEquip(weaponToAdd->_weaponSlot);
	// Otherwise if the currently equipped weapon has no ammo, equip this new one
	else if (GetCurrentWeaponAmmoCount() == 0)
		TryEquip(weaponToAdd->_weaponSlot);

	_onWeaponIsAddedDelegate.Broadcast(weaponToAdd);

	if (addAmmo)
		TryAddAmmo(weaponToAdd->_ammoType, ammoCount);

	return true;
}

bool UInventoryComponent::TryAddAmmo(UAmmoType* ammoToAdd, int ammoCount)
{
	if (!ammoToAdd)
		return false;

	// Check if the ammo type was never added to the inventory, in case, add it
	if (!_ammo.Contains(ammoToAdd))
		_ammo.Add(ammoToAdd, 0);

	// Check if it's already max ammo
	if (_ammo[ammoToAdd] >= ammoToAdd->_maxCapacity)
		return false;

	// Add ammo count
	_ammo[ammoToAdd] = FMath::Clamp(_ammo[ammoToAdd] + ammoCount, 0, ammoToAdd->_maxCapacity);

	_onAmmoIsAddedDelegate.Broadcast(ammoToAdd);
	return true;
}

bool UInventoryComponent::TryEquip(EWeaponSlot slot)
{
	// Spawn WeaponInstance (may want to cache them for weapon switch)
	_weapons[slot]._instance = GetWorld()->SpawnActor<AWeaponInstance>(_weapons[slot]._asset->_weaponActor, this->GetOwner()->GetTransform());
	_weapons[slot]._instance->SetWeaponOwner(this->GetOwner());

	// Attach to owner
	FAttachmentTransformRules attachmentRules(EAttachmentRule::SnapToTarget, true);
	_weapons[slot]._instance->AttachToActor(_weapons[slot]._instance->GetWeaponOwner(), attachmentRules);

	_onWeaponIsEquippedDelegate.Broadcast(_weapons[slot]);
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
