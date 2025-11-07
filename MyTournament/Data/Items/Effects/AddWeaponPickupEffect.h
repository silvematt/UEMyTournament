// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../PickupEffect.h"
#include "AddWeaponPickupEffect.generated.h"

class UWeaponAsset;

/**
 * 
 */
UCLASS(BlueprintType, EditInlineNew)
class MYTOURNAMENT_API UAddWeaponPickupEffect : public UPickupEffect
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UWeaponAsset> _weaponToAdd;

	UPROPERTY(EditAnywhere)
	bool _addAmmo = true;

	UPROPERTY(EditAnywhere)
	int _ammoCount = 30;

	UFUNCTION()
	bool ApplyTo_Implementation(AActor* taker) override;
};
