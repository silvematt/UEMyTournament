// Fill out your copyright notice in the Description page of Project Settings.


#include "AddArmorPickupEffect.h"
#include "../../../Actors/Components/EntityVitalsComponent.h"
#include "../../../Interfaces/Damageable.h"

bool UAddArmorPickupEffect::ApplyTo_Implementation(AActor* taker)
{
	if (taker)
	{
		// Check if the taker has the EntityVitalsComponent, which implements the IDamageable interface
		if (auto* comp = taker->FindComponentByClass<UEntityVitalsComponent>())
		{
			IDamageable::Execute_AddArmor(comp, _amount);
			return true;
		}

		// Check if the actor itself implements the IDamageable interface
		if (taker->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
		{
			IDamageable::Execute_AddArmor(taker, _amount);
			return true;
		}
	}

	return false;
}
