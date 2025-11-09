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
	Slot3 = 3
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
	EFireMode _fireMode = EFireMode::Single;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core")
	EShootingType _shootingType = EShootingType::Raycast;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core")
	float _damage = 25.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core", meta = (ClampMin = "0.1"))
	float _fireRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core")
	float _range = 10000.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile | Settings")
	float _impulseForceOnHit = 10000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Core")
	TObjectPtr<UAmmoType> _ammoType;
};
