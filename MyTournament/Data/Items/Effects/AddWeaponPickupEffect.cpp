// Fill out your copyright notice in the Description page of Project Settings.


#include "AddWeaponPickupEffect.h"
#include "../../../Player/MyTournamentCharacter.h"
#include "../../../Actors/Components/InventoryComponent.h"

bool UAddWeaponPickupEffect::ApplyTo_Implementation(AActor* taker)
{
	if (taker)
	{
		// Check if its the player
		if (auto* comp = taker->FindComponentByClass<UInventoryComponent>())
		{
			comp->TryAddWeapon(_weaponToAdd, _ammoCount);
		}
	}

	return false;
}
