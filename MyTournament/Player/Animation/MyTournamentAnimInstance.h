// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MyTournamentAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MYTOURNAMENT_API UMyTournamentAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	bool _bIsDashing = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	bool _bIsShooting = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	bool _bIsWeaponDrawn = false;

	// Resets the custom properties to default, useful when switching AnimInstances
	UFUNCTION()
	void ResetProperties();
};
