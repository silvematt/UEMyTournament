// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.h"
#include "PickupEffect.h"
#include "ItemAsset.generated.h"

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

	UPROPERTY(EditAnywhere, Category = "Effects")
	TArray<TObjectPtr<UPickupEffect>> _effects;

};
