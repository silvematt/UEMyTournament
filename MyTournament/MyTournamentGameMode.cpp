// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTournamentGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/Items/PickupBase.h"


void AMyTournamentGameMode::StartPlay()
{
	Super::StartPlay();

	check(GEngine != nullptr);
	GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("MyTournament Game Mode started!"));
}

void AMyTournamentGameMode::RegisterHealthPickup(APickupBase* toRegister)
{
	if(toRegister)
		_healthPickups.Add(toRegister);
}

void AMyTournamentGameMode::UnregisterHealthPickup(APickupBase* toRemove)
{
	if (toRemove)
		_healthPickups.Remove(toRemove);
}

APickupBase* AMyTournamentGameMode::GetBestHealthPickup(const FVector& fromLoc) const
{
	float bestDist = FLT_MAX;
	APickupBase* bestPickup = nullptr;

	for (int i = 0; i < _healthPickups.Num(); i++)
	{
		if (_healthPickups[i]->IsCurrentlyActive())
		{
			float curDist = FVector::DistSquared(fromLoc, _healthPickups[i]->GetActorLocation());

			if (curDist < bestDist)
			{
				bestDist = curDist;
				bestPickup = _healthPickups[i];
			}
		}
	}

	return bestPickup;
}
