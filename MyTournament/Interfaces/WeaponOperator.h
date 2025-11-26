// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponOperator.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWeaponOperator : public UInterface
{
	GENERATED_BODY()
};

/**
 *  This interface is used by any entity that can operate and shoot weapons (player/ai).
 */
class MYTOURNAMENT_API IWeaponOperator
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// Returns a calculated direction for the next bullet to be shot by this WeaponOperator
	// For the player it can be the center of the camera
	// For the AI it can be a calculation with noise
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FVector GetAimPoint();

	// Returns whether or not the weapon operator is aiming downsight
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsAimingDownsight();
};
