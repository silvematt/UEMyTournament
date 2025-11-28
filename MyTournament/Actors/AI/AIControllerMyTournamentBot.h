// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIControllerMyTournamentBot.generated.h"

class UBrainComponent;

/**
 * 
 */
UCLASS()
class MYTOURNAMENT_API AAIControllerMyTournamentBot : public AAIController
{
	GENERATED_BODY()

protected:
	UBrainComponent* _myBrain = nullptr;
	
public:
	// Called by the AIBot when it dies
	UFUNCTION()
	void OnControlledCharacterDeath();
};
