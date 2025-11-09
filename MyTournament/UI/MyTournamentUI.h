// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyTournamentUI.generated.h"

class UTextBlock;
class UProgressBar;
class UOverlay;
class UImage;
class UEntityVitalsComponent;
class UInventoryComponent;
struct FWeaponInInventoryEntry;

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

	// WeaponPanel
	UPROPERTY(meta = (BindWidget))
	UOverlay* _weaponOverlay;

	UPROPERTY(meta = (BindWidget))
	UImage* _weaponIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* _ammoIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* _ammoCountValue;

	// Other
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UEntityVitalsComponent> _playerVitals;

	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UInventoryComponent> _playerInventory;

// Methods
protected:
	void NativeConstruct() override;

	UFUNCTION()
	void HandleDashChange(int curDashAvailable);

	UFUNCTION()
	void HandleOnVitalsChange(float newHP, float newArmor);

	UFUNCTION()
	void HandleOnWeaponIsEquipped(const FWeaponInInventoryEntry& newWeapon);

	UFUNCTION()
	void HandleOnAmmoIsAdded(UAmmoType* newAmmo);

	// Updates the UI weapon counter
	UFUNCTION()
	void HandleOnAmmoIsConsumed(UAmmoType* newAmmo, uint32 ammoVal);

public:
	// CustomInitialize is called by the owner of this component (like the player)
	UFUNCTION()
	void CustomInitialize();
};
