// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Actors/Components/EntityVitalsComponent.h"
#include "Actors/Characters/MyTournamentCharacterBase.h"
#include "AIMyTournamentBot.generated.h"

class AAIControllerMyTournamentBot;
class UInventoryComponent;
class UMyTournamentAnimInstance;

UCLASS()
class MYTOURNAMENT_API AAIMyTournamentBot : public AMyTournamentCharacterBase
{
	GENERATED_BODY()

// Properties
protected:
	UPROPERTY()
	TObjectPtr<AAIControllerMyTournamentBot> _myController;

// Methods
public:
	// Sets default values for this character's properties
	AAIMyTournamentBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called by _entityVitals's delegate onVitalsChange
	UFUNCTION()
	void HandleOnVitalsChange(float newHP, float newAP);

	// Allows to have a BP_ function to run when HandleOnVitalsChange gets called by _entityVitals's delegate onVitalsChange
	UFUNCTION(BlueprintImplementableEvent, Category = "Vitals")
	void BP_OnVitalsChange(float NewHP, float NewAP);

	// Called by _entityVitals's delegate onDeathDelegate
	UFUNCTION()
	void HandleOnDeath();

	// Pure virtuals overrides
	virtual bool CanDash() override;

	virtual void HandleOnWeaponEquipped(const FWeaponInInventoryEntry& weaponEntry) override;
	virtual void HandleOnWeaponUnequipped(const FWeaponInInventoryEntry& weaponEntry) override;
};
