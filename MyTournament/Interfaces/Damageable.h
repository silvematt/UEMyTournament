// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MYTOURNAMENT_API IDamageable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ApplyDamage(float dmgApplied);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AddHealth(float hpAmount);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AddArmor(float armorAmount);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsAtMaxHealth() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsAtMaxArmor() const;
};

