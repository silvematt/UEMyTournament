// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Components/EntityVitalsComponent.h"
#include "AIMyTournamentBot.generated.h"

class AAIControllerMyTournamentBot;

UCLASS()
class MYTOURNAMENT_API AAIMyTournamentBot : public ACharacter
{
	GENERATED_BODY()

// Properties
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UEntityVitalsComponent> _entityVitals;

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

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called by _entityVitals's delegate onVitalsChange
	UFUNCTION()
	void HandleOnVitalsChange(float newHP, float newAP);

	// Called by _entityVitals's delegate onDeathDelegate
	UFUNCTION()
	void HandleOnDeath();

	// Allows to have a BP_ function to run when HandleOnVitalsChange gets called by _entityVitals's delegate onVitalsChange
	UFUNCTION(BlueprintImplementableEvent, Category = "Vitals")
	void BP_OnVitalsChange(float NewHP, float NewAP);
};
