// Fill out your copyright notice in the Description page of Project Settings.


#include "AIControllerMyTournamentBot.h"
#include "BrainComponent.h"

void AAIControllerMyTournamentBot::OnControlledCharacterDeath()
{
	GetBrainComponent()->StopLogic(TEXT("Character Died."));
}
