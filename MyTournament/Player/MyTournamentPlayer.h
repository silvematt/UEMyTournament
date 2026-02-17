// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Characters/MyTournamentCharacterBase.h"
#include "MyTournamentPlayer.generated.h"

/**
 * 
 */
UCLASS()
class MYTOURNAMENT_API AMyTournamentPlayer : public AMyTournamentCharacterBase
{
	GENERATED_BODY()
	
// Properties
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | Input")
	TObjectPtr<UInputMappingContext> _baseInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | Input")
	TObjectPtr<UInputAction> _moveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | Input")
	TObjectPtr<UInputAction> _jumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | Input")
	TObjectPtr<UInputAction> _lookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | Input")
	TObjectPtr<UInputAction> _crouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | Input")
	TObjectPtr<UInputAction> _firePrimaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | Input")
	TObjectPtr<UInputAction> _fireSecondaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "	CharacterBase | Player | Input")
	TObjectPtr<UInputAction> _dashAction;

	UPROPERTY(VisibleAnywhere, Category = "CharacterBase | Player | Input")
	float _normalLookSensitivity = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "CharacterBase | Player | Input")
	float _aimingDownsightLookSensitivity = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | Dash | Behavior")
	float _dashInputThreshold = 0.25f; // _movementVector.Length() has to be >= than _dashInputThreshold for the dash to be able to happen


	// Components

	// - Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | Camera")
	TObjectPtr<UCameraComponent> _cameraComponent;

	UPROPERTY(EditAnywhere, Category = "CharacterBase | Player | Camera")
	float _cameraFOV = 90.0f;

	UPROPERTY(EditAnywhere, Category = "CharacterBase | Player | Camera")
	float _cameraScale = 0.6f;

	// - Camera Holder
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | Camera")
	TObjectPtr<USceneComponent> _cameraHolder;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "CharacterBase | Player | Camera")
	FTransform _fpsMeshHeadSocket; // to set _cameraHolder position always to be on the head socket

	// - Animation
	UPROPERTY(VisibleAnywhere, Category = "CharacterBase | Player | Animation")
	TObjectPtr<UMyTournamentAnimInstance> _fpsAnimInstance;

	UPROPERTY(EditAnywhere, Category = "CharacterBase | Player | Animation")
	TObjectPtr<UAnimBlueprint> _fpsDefaultAnim;

	// - Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | First Person")
	TObjectPtr<USkeletalMeshComponent> _fpsMesh;

	// - HUD
	UPROPERTY(EditAnywhere, Category = "CharacterBase | Player | HUD")
	TSubclassOf<UMyTournamentUI> _myTournamentUIClass;

	UPROPERTY()
	TObjectPtr<UMyTournamentUI> _myTournamentUI;

	UPROPERTY()
	TObjectPtr<UUserWidget> _currentAimingDownsightUW;

	// Wall Run
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | WallRun | Settings")
	float _wallRunCameraTiltValue = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Player | WallRun | Settings")
	float _wallRunCameraTiltInterpSpeed = 5.0f;

// Methods
public:
	AMyTournamentPlayer();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void Move(const FInputActionValue& inputValue);

	UFUNCTION()
	void StopMoving();

	UFUNCTION()
	void Look(const FInputActionValue& inputValue);

	virtual void PerformDash() override;

	// During WallRun
	UFUNCTION()
	void UpdateCameraTiltDuringWallrun(float deltaTime);

	// Called by _entityVitals's delegate onVitalsChange
	UFUNCTION()
	void HandleOnVitalsChange(float newHP, float newAP);

	// Allows to have a BP_ function to run when HandleOnVitalsChange gets called by _entityVitals's delegate onVitalsChange
	UFUNCTION(BlueprintImplementableEvent, Category = "Vitals")
	void BP_OnVitalsChange(float NewHP, float NewAP);

	// Called by _entityVitals's delegate onDeathDelegate
	UFUNCTION()
	void HandleOnDeath();


	virtual FRotator BPF_GetWorldRotation() override;

	// IWeaponInterface
	virtual FVector GetAimPoint_Implementation() override;

	// Pure Virtual overrides
	virtual bool CanDash() override;

	virtual void HandleOnWeaponEquipped(const FWeaponInInventoryEntry& weaponEntry) override;
	virtual void HandleOnWeaponUnequipped(const FWeaponInInventoryEntry& weaponEntry) override;

	virtual void HandleOnWeaponFirePrimary() override;
	virtual void HandleOnWeaponFireSecondary() override;

	virtual void AimDownsight() override;
	virtual void StopAimingDownsight() override;
};
