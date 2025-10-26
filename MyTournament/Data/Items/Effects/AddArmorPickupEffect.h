// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../PickupEffect.h"
#include "AddArmorPickupEffect.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew)
class MYTOURNAMENT_API UAddArmorPickupEffect : public UPickupEffect
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	float _amount = 20.0f;

	UFUNCTION()
	bool ApplyTo_Implementation(AActor* taker) override;
};
