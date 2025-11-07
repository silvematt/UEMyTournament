// Fill out your copyright notice in the Description page of Project Settings.


#include "AddAmmoPickupEffect.h"
#include "../../../Player/MyTournamentCharacter.h"
#include "../../../Actors/Components/InventoryComponent.h"

bool UAddAmmoPickupEffect::ApplyTo_Implementation(AActor* taker)
{
	if (taker)
	{
		// Check if its the player
		if (auto* comp = taker->FindComponentByClass<UInventoryComponent>())
		{
			comp->TryAddAmmo(_ammoToAdd, _ammoCount);
		}
	}

	return false;
}
