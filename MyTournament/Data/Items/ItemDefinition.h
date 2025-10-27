// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDefinition.generated.h"

class UItemAsset;

UENUM()
enum class EItemType : uint8
{
	Consumable	UMETA(DisplayName = "Consumable"),
	Powerup		UMETA(DisplayName = "Powerup"),
	Ammo		UMETA(DisplayName = "Ammo"),
	Weapon		UMETA(DisplayName = "Weapon")
};

USTRUCT()
struct FItemText
{
	GENERATED_BODY()	

	UPROPERTY(EditAnywhere)
	FText itemName;

	UPROPERTY(EditAnywhere)
	FText itemDescription;
};

USTRUCT()
struct FItemDefinition : public FTableRowBase
{
	GENERATED_BODY()

// Properties
private:
	UPROPERTY(EditAnywhere, Category = "Item Data")
	TObjectPtr<UItemAsset> _itemAsset;
};
