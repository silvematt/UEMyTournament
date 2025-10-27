// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.h"
#include "PickupEffect.h"
#include "ItemAsset.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EPickupCondition : uint8
{
	None									= 0 UMETA(Hidden),
	DoNotTake_OnHealthIsMax					= 1 << 0,
	DoNotTake_OnArmorIsMax					= 1 << 1,
	DoNotTake_OnBothHealthAndArmorIsMax		= 1 << 2,
	DoNotTake_OnAmmoTypeIsFull				= 1 << 3
};
ENUM_CLASS_FLAGS(EPickupCondition)

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MYTOURNAMENT_API UItemAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FText _ID;

	UPROPERTY(EditAnywhere, Category = "Item Data")
	EItemType _type;

	UPROPERTY(EditAnywhere, Category = "Item Data")
	FItemText _textData;

	// The Static Mesh used to display this item in the world.
	UPROPERTY(EditAnywhere, Category = "Item Data")
	TSoftObjectPtr<UStaticMesh> _meshPrimary;

	UPROPERTY(EditAnywhere, Category = "Pickup Data")
	TArray<TObjectPtr<UPickupEffect>> _effects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Data", meta = (Bitmask, BitmaskEnum = "EPickupCondition"))
	int32 _pickupConditionsMask = 0;

public:
	FORCEINLINE bool HasPickupCondition(EPickupCondition Flag) const
	{
		return (_pickupConditionsMask & static_cast<int32>(Flag)) != 0;
	}
};
