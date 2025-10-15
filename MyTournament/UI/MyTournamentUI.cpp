// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTournamentUI.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include <Kismet/GameplayStatics.h>
#include "../Player/MyTournamentCharacter.h"

void UMyTournamentUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (_txtHPValue)
	{
		_txtHPValue->SetText(FText::FromString("OVERRIDE"));
	}

	AMyTournamentCharacter* myCharacter = Cast<AMyTournamentCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Bind to delegates
	// Dashes
	if (myCharacter)
	{
		myCharacter->_onDashIsUsedDelegate.AddUniqueDynamic(this, &UMyTournamentUI::HandleDashChange);
		myCharacter->_onDashIsRefilledDelegate.AddUniqueDynamic(this, &UMyTournamentUI::HandleDashChange);
	}
}
void UMyTournamentUI::HandleDashChange(int curDashAvailable)
{
	switch (curDashAvailable)
	{
		case 0:
			_dashBarOne->SetPercent(0.0f);
			_dashBarTwo->SetPercent(0.0f);
			break;

		case 1:
			_dashBarOne->SetPercent(1.0f);
			_dashBarTwo->SetPercent(0.0f);
			break;

		case 2:
			_dashBarOne->SetPercent(1.0f);
			_dashBarTwo->SetPercent(1.0f);
			break;

		default:
			break;
	}
}
