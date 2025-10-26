// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyTournamentUI.generated.h"

class UTextBlock;
class UProgressBar;
class UEntityVitalsComponent;

/**
 * 
 */
UCLASS(abstract)
class MYTOURNAMENT_API UMyTournamentUI : public UUserWidget
{
	GENERATED_BODY()

// Properties
protected:
	// UI
	UPROPERTY(meta = (BindWidget))
	UTextBlock* _txtHPValue;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* _txtArmorValue;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* _dashBarOne;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* _dashBarTwo;

	// Other
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UEntityVitalsComponent> _playerVitals;

// Methods
protected:
	void NativeConstruct() override;

	UFUNCTION()
	void HandleDashChange(int curDashAvailable);

	UFUNCTION()
	void HandleOnVitalsChange(float newHP, float newArmor);
};
