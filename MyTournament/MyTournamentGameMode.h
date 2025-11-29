// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyTournamentGameMode.generated.h"

class APickupBase;

/**
 * 
 */
UCLASS()
class MYTOURNAMENT_API AMyTournamentGameMode : public AGameModeBase
{
	GENERATED_BODY()

// Properties
protected:
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<APickupBase>> _healthPickups;

// Methods
protected:	
	virtual void StartPlay() override;

public:
	void RegisterHealthPickup(APickupBase* toRegister);
	void UnregisterHealthPickup(APickupBase* toRemove);

	UFUNCTION(BlueprintCallable)
	APickupBase* GetBestHealthPickup(const FVector& fromLoc) const;
};
