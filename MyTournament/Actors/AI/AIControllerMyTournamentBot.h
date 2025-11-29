// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIControllerMyTournamentBot.generated.h"

class UBrainComponent;
class UBlackboardComponent;

/**
 * 
 */
UCLASS()
class MYTOURNAMENT_API AAIControllerMyTournamentBot : public AAIController
{
	GENERATED_BODY()

public:
	// Called by the AIBot when it dies
	UFUNCTION()
	void OnControlledCharacterDeath();

};
