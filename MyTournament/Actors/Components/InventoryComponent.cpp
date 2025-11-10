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
		//TryAddWeapon(_defaultWeapon, _defaultWeaponAmmoCount);
	}
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UInventoryComponent::TryAddWeapon(UWeaponAsset* weaponToAdd, uint32 ammoCount)
{
	if (!weaponToAdd) 
		return false;

	// Check if the weapon was never added to the inventory, in case, add it
	if (!_weapons.Contains(weaponToAdd->_weaponSlot))
		_weapons.Add(weaponToAdd->_weaponSlot, FWeaponInInventoryEntry(weaponToAdd));

	// Always add ammo first, even if ammoCount is 0, the AmmoType entry will be created
	TryAddAmmo(weaponToAdd->_ammoType, ammoCount);

	// If no weapon was equipped, equip this one
	if (_currentWeaponSlot == EWeaponSlot::Slot0)
		TryEquip(weaponToAdd->_weaponSlot);
	// Otherwise if the currently equipped weapon has no ammo (and it's a different weapon), equip this new one
	else if (_currentWeaponSlot != weaponToAdd->_weaponSlot && GetCurrentWeaponAmmoCount() == 0)
		TryEquip(weaponToAdd->_weaponSlot);

	_onWeaponIsAddedDelegate.Broadcast(weaponToAdd);

	return true;
}

bool UInventoryComponent::TryAddAmmo(UAmmoType* ammoToAdd, uint32 ammoCount)
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
	if (_currentWeaponSlot != EWeaponSlot::Slot0)
		UnequipCurrentWeapon();

	// Spawn WeaponInstance (may want to cache them for weapon switch)
	_weapons[slot]._instance = GetWorld()->SpawnActor<AWeaponInstance>(_weapons[slot]._asset->_weaponActor, this->GetOwner()->GetTransform());
	_weapons[slot]._instance->SetWeaponOwner(this->GetOwner());

	// Attach to owner
	FAttachmentTransformRules attachmentRules(EAttachmentRule::SnapToTarget, true);
	_weapons[slot]._instance->AttachToActor(_weapons[slot]._instance->GetWeaponOwner(), attachmentRules);

	// Update _currentWeaponSlot
	_currentWeaponSlot = slot;

	_onWeaponIsEquippedDelegate.Broadcast(_weapons[slot]);
	return true;
}

int32 UInventoryComponent::GetCurrentWeaponAmmoCount() const
{
	if (_currentWeaponSlot == EWeaponSlot::Slot0)
		return 0;

	// Check for invalid case
	const FWeaponInInventoryEntry* entry = _weapons.Find(_currentWeaponSlot);
	if (!entry)
		return 0;

	const UAmmoType* ammoType = entry->_asset->_ammoType;
	const int32 ammoCount = _ammo.FindRef(ammoType); // returns 0 if not found
	return ammoCount;
}

uint32 UInventoryComponent::GetAmmoCount(UAmmoType* ammo) 
{
	if (_currentWeaponSlot == EWeaponSlot::Slot0)
		return 0;

	if (!_ammo.Contains(ammo))
		return 0;

	return _ammo[ammo];
}

void UInventoryComponent::ConsumeAmmo(UAmmoType* ammo, uint32 val)
{
	if (_currentWeaponSlot == EWeaponSlot::Slot0)
		return;

	if (!_ammo.Contains(ammo))
		return;

	_ammo[ammo] = FMath::Clamp(_ammo[ammo] - val, 0, ammo->_maxCapacity);

	_onAmmoIsConsumedDelegate.Broadcast(ammo, _ammo[ammo]);
}

bool UInventoryComponent::UnequipCurrentWeapon()
{

	_onWeaponIsUnequippedDelegate.Broadcast(_weapons[_currentWeaponSlot]);
	return true;
}
