// Fill out your copyright notice in the Description page of Project Settings.


#include "HealPickupEffect.h"
#include "../../../Actors/Components/EntityVitalsComponent.h"
#include "../../../Interfaces/Damageable.h"

bool UHealPickupEffect::ApplyTo_Implementation(AActor* taker)
{
	if (taker)
	{
		// Check if the taker has the EntityVitalsComponent, which implements the IDamageable interface
		if (auto* comp = taker->FindComponentByClass<UEntityVitalsComponent>())
		{
			IDamageable::Execute_AddHealth(comp, _amount);
			return true;
		}

		// Check if the actor itself implements the IDamageable interface
		if (taker->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
		{
			IDamageable::Execute_AddHealth(taker, _amount);
			return true;
		}
	}

	return false;
}
