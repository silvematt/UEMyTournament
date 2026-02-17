// Fill out your copyright notice in the Description page of Project Settings.


#include "AddWeaponPickupEffect.h"
#include "Actors/Characters/MyTournamentCharacterBase.h"
#include "Actors/Components/InventoryComponent.h"

bool UAddWeaponPickupEffect::ApplyTo_Implementation(AActor* taker)
{
	TArray<UInventoryComponent*> Inventories;
	taker->GetComponents<UInventoryComponent>(Inventories);

	if (Inventories.Num() > 1)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("DUPLICATE FOUND! Bot has %d inventories."), Inventories.Num()));
	}

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
