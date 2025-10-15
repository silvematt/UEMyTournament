// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyTournamentUI.generated.h"

class UTextBlock;
class UProgressBar;

/**
 * 
 */
UCLASS(abstract)
class MYTOURNAMENT_API UMyTournamentUI : public UUserWidget
{
	GENERATED_BODY()

// Properties
protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* _txtHPValue;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* _dashBarOne;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* _dashBarTwo;

// Methods
protected:
	void NativeConstruct() override;

	UFUNCTION()
	void HandleDashChange(int curDashAvailable);
};
