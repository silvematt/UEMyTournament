// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AmmoType.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MYTOURNAMENT_API UAmmoType : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	FName _ID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSoftObjectPtr<UTexture2D> _ammoIcon;

	UPROPERTY(EditDefaultsOnly)
	FText _displayName;

	UPROPERTY(EditDefaultsOnly)
	uint32 _maxCapacity;	
};
