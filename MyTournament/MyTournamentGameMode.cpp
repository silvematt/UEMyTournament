// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTournamentGameMode.h"
#include "Kismet/GameplayStatics.h"

void AMyTournamentGameMode::StartPlay()
{
	Super::StartPlay();

	check(GEngine != nullptr);
	GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("MyTournament Game Mode started!"));
}
