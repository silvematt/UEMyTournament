// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../PickupEffect.h"
#include "AddAmmoPickupEffect.generated.h"


class UAmmoType;

/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew)
class MYTOURNAMENT_API UAddAmmoPickupEffect : public UPickupEffect
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAmmoType> _ammoToAdd;

	UPROPERTY(EditAnywhere)
	int _ammoCount = 30;

	UFUNCTION()
	bool ApplyTo_Implementation(AActor* taker) override;
};
