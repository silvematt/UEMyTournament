// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PickupEffect.generated.h"

/**
 *  Base class for a PickupEffect
 * 
 * PickupEfect will be implemented as classed that derives from UPickupEffect.
 * A pickup blueprint will be able to have a list of UPickupEffect to dinamically assign effects to pickups
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class MYTOURNAMENT_API UPickupEffect : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool ApplyTo(AActor* taker); // apply effect to taker Actor 

	virtual bool ApplyTo_Implementation(AActor* Taker) { return false; }
};
