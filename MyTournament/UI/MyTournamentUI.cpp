// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTournamentUI.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include <Kismet/GameplayStatics.h>
#include "../Player/MyTournamentCharacter.h"
#include "../Actors/Components/EntityVitalsComponent.h"
#include "../Actors/Components/InventoryComponent.h"
#include "../Actors/Items/WeaponInstance.h"

void UMyTournamentUI::NativeConstruct()
{
	Super::NativeConstruct();
	
}

// CustomInitialize is called by the owner of this component (like the player)
void UMyTournamentUI::CustomInitialize()
{
	// Initialize UI 
	_weaponOverlay->SetVisibility(ESlateVisibility::Hidden);

	AMyTournamentCharacter* myCharacter = Cast<AMyTournamentCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Bind to delegates
	// Dashes
	if (myCharacter)
	{
		myCharacter->_onDashIsUsedDelegate.AddUniqueDynamic(this, &UMyTournamentUI::HandleDashChange);
		myCharacter->_onDashIsRefilledDelegate.AddUniqueDynamic(this, &UMyTournamentUI::HandleDashChange);

		_playerVitals = myCharacter->GetVitalsComponent();
		_playerInventory = myCharacter->GetInventoryComponent();

		if (_playerVitals)
		{
			_playerVitals->_onVitalsChange.AddUniqueDynamic(this, &UMyTournamentUI::HandleOnVitalsChange);

			// Do one update
			HandleOnVitalsChange(_playerVitals->GetCurrentHealth(), _playerVitals->GetCurrentArmor());
		}

		if (_playerInventory)
		{
			_playerInventory->_onWeaponIsEquippedDelegate.AddDynamic(this, &UMyTournamentUI::HandleOnWeaponIsEquipped);
			_playerInventory->_onAmmoIsAddedDelegate.AddDynamic(this, &UMyTournamentUI::HandleOnAmmoIsAdded);
			_playerInventory->_onAmmoIsConsumedDelegate.AddDynamic(this, &UMyTournamentUI::HandleOnAmmoIsConsumed);
		}
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

void UMyTournamentUI::HandleOnVitalsChange(float newHP, float newArmor)
{
	_txtHPValue->SetText(FText::AsNumber(FMath::RoundToInt(newHP)));
	_txtArmorValue->SetText(FText::AsNumber(FMath::RoundToInt(newArmor)));
}

void UMyTournamentUI::HandleOnWeaponIsEquipped(const FWeaponInInventoryEntry& newWeapon)
{
	if (_weaponOverlay->GetVisibility() == ESlateVisibility::Hidden)
		_weaponOverlay->SetVisibility(ESlateVisibility::Visible);

	// Attempt to load weapon icon
	auto& wIcon = newWeapon._asset->_weaponIcon;
	if (!wIcon.IsNull())
	{
		if (!wIcon.IsValid())
			wIcon.LoadSynchronous();

		_weaponIcon->SetBrushFromTexture(wIcon.Get());
	}

	// Attempt to load ammo icon
	auto& aIcon = newWeapon._asset->_ammoType->_ammoIcon;
	if (!aIcon.IsNull())
	{
		if (!aIcon.IsValid())
			aIcon.LoadSynchronous();

		_ammoIcon->SetBrushFromTexture(aIcon.Get());
	}

	_ammoCountValue->SetText(FText::AsNumber(_playerInventory->GetCurrentWeaponAmmoCount()));
}

void UMyTournamentUI::HandleOnAmmoIsAdded(UAmmoType* newAmmo)
{
	// Attempt to load ammo icon
	auto& aIcon = newAmmo->_ammoIcon;
	if (!aIcon.IsNull())
		if (!aIcon.IsValid())
		{
			aIcon.LoadSynchronous();
			_ammoIcon->SetBrushFromTexture(aIcon.Get());
		}

	_ammoCountValue->SetText(FText::AsNumber(_playerInventory->GetCurrentWeaponAmmoCount()));
}

void UMyTournamentUI::HandleOnAmmoIsConsumed(UAmmoType* newAmmo, uint32 ammoVal)
{
	_ammoCountValue->SetText(FText::AsNumber(_playerInventory->GetCurrentWeaponAmmoCount()));
}