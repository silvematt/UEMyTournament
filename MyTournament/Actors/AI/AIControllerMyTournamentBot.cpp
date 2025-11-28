// Fill out your copyright notice in the Description page of Project Settings.


#include "AIControllerMyTournamentBot.h"
#include "BrainComponent.h"

void AAIControllerMyTournamentBot::OnControlledCharacterDeath()
{
	if (!_myBrain)
		_myBrain = GetBrainComponent();

	_myBrain->StopLogic(TEXT("Character Died."));
}
