// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemAsset.h"
#include "Weapons/AmmoType.h"
#include "WeaponAsset.generated.h"

class AWeaponInstance;

UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	Slot0 = 0 UMETA(Hidden), //Slot0 means not-initialized
	Slot1 = 1, 
	Slot2 = 2,
	Slot3 = 3,
	Slot4 = 4
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Single = 0,
	Burst,
	Auto
};

UENUM(BlueprintType)
enum class EShootingType : uint8
{
	Raycast = 0,
	Bullet,
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MYTOURNAMENT_API UWeaponAsset : public UItemAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core")
	EWeaponSlot _weaponSlot = EWeaponSlot::Slot1;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core")
	TSubclassOf<AWeaponInstance> _weaponActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSoftObjectPtr<UTexture2D> _weaponIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core")
	EShootingType _shootingType = EShootingType::Raycast;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core", meta = (EditCondition = "_shootingType == EShootingType::Raycast", EditConditionHides))
	TSubclassOf<AActor> _impactVFX;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core")
	EFireMode _fireMode = EFireMode::Single;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core", meta = (EditCondition = "_fireMode == EFireMode::Burst", EditConditionHides))
	int _burstShotsToFire = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core", meta = (EditCondition = "_fireMode == EFireMode::Burst", EditConditionHides))
	float _burstFireRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core")
	float _damage = 25.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core", meta = (ClampMin = "0.1"))
	float _fireRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core")
	float _range = 10000.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon | Core")
	float _impulseForceOnHit = 10000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core")
	TObjectPtr<UAmmoType> _ammoType;

	// Aim Downsight as fire secondary
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool _aimsDownsightAsSecondaryFire = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> _aimDownsightUWClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float _aimDownsightFOV = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool _weaponUsesProgressiveSpread = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "_weaponUsesProgressiveSpread == true", EditConditionHides))
	float _weaponProgressiveSpreadIncreaseRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "_weaponUsesProgressiveSpread == true", EditConditionHides))
	float _weaponProgressiveSpreadCooldownRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float _weaponSpreadHipfire = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float _weaponSpreadAimfire = 0.0f;
};
