// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "Interfaces/WeaponOperator.h"
#include "MyTournamentCharacterBase.generated.h"

class UInputMappingContext;
class UInputAction;
class UInputComponent;
class UAnimBlueprint;
class UMyTournamentUI;
class UMyTournamentAnimInstance;
class UEntityVitalsComponent;
class UInventoryComponent;
struct FWeaponInInventoryEntry;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDashIsUsedSignature, int, dashesNowAvailable);

// The wallrun modes
UENUM()
enum class EWallRunModes : uint8
{
	None = 0,
	LeftWall = 1, // 1 referenced in anim blueprint
	RightWall = 2, // 2 referenced in anim blueprint
	VerticalWall = 3 // 3 is referenced in the anim blueprint
};

/*
AMyTournamentCharacterBase needs a C++ derived class (Player/Bot)
*/
UCLASS(Abstract)
class MYTOURNAMENT_API AMyTournamentCharacterBase : public ACharacter, public IWeaponOperator
{
	GENERATED_BODY()

// Properties
protected:

	// Input
	UPROPERTY(VisibleAnywhere, Category = "CharacterBase | Player | Input")
	FVector2D _movementVector;

	// Dash
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Dash | Behavior")
	float _dashForce = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Dash | Behavior")
	float _dashVerticalLift = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Dash | Behavior")
	float _dashGroundedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Dash | Behavior")
	float _dashAirboneMultiplier = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Dash | Behavior")
	int _dashAvailableNum = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Dash | Behavior")
	float _dashRestoreCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterBase | Dash | State")
	int _dashCurAvaiable = 2;


	// Components

	// - Animation
	UPROPERTY(VisibleAnywhere, Category = "Animation")
	TObjectPtr<UMyTournamentAnimInstance> _characterAnimInstance;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimBlueprint> _tpsDefaultAnim;

	// - Character Component (inerithed)
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UCharacterMovementComponent> _movementComponent;

	// Vitals
	UPROPERTY(VisibleAnywhere, Category = "Vitals")
	TObjectPtr<UEntityVitalsComponent> _vitalsComponent;

	// WallRunning
	// - Settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Settings")
	bool _wallRunDebugEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Settings")
	float _wallRunMinVelocityStartup = 550.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Settings")
	float _wallRunInputThreshold = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Settings")
	float _wallLateralRunCheckVectorLength = 40.0f; // lenght of the vector to check for walls on the left and right of the player

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Settings")
	float _wallVerticalRunCheckVectorLength = 80.0f; // lenght of the vector to check for vertical walls

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Settings")
	float _wallRunStickCheckVectorLength = 80.0f;	// Once the wall run has started, this is the lenght of the vector that checks if we're still attached to the wall
													// it's bigger than _wallRunCheckVectorLength because 1: we want _wallRunCheckVectorLength to be small for precise input, 
													// 2: if the "stick-check" has the vector length too small, if we're sliding along a rounded surface it may fail

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Settings")
	float _wallRunForwardObstacleCheckLength = 50.0f; // While lateral wallruning, we check if there's an object in front of us that would be an obstacle to our wallrun. If so, the wallrun will be stopped.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Settings")
	float _wallRunVerticalObstacleCheckLength = 100.0f; // While vertical wallruning, we check if there's an object on top of our head that would be an obstacle to our wallrun. If so, the wallrun will be stopped.

	// - Behavior
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Behavior")
	float _wallRunLateralGravityScaleModifier = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Behavior")
	float _wallRunVerticalGravityScaleModifier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Behavior")
	float _lateralWallRunSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Behavior")
	float _verticalWallRunSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Behavior")
	float _verticalWallRunLaunchForceToClimb = 500.0f; // when a wallrun is completed (or cancelled by looking away) we can automatically launch the character with a little bit of force in order to get up and climb the wall

	// - State
	UPROPERTY(VisibleAnywhere, Category = "WallRun | State")
	bool _wallRunIsWallRunning = false;

	UPROPERTY(VisibleAnywhere, Category = "WallRun | State")
	EWallRunModes _curWallRunMode = EWallRunModes::None;

	UPROPERTY(VisibleAnywhere, Category = "WallRun | State")
	float _wallRunTimeWallRunning = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "WallRun | State")
	FVector _wallRunCurWallNormal;

	UPROPERTY(VisibleAnywhere, Category = "WallRun | State")
	FVector _wallRunCurWallImpactPoint;

	UPROPERTY(VisibleAnywhere, Category = "WallRun | State")
	TObjectPtr<AActor> _wallRunLastWall; // last wall we ran on, while wallrunning it is the wall we're currently wallrunning

	// Inventory
	UPROPERTY(VisibleDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInventoryComponent> _inventoryComponent;

	// Other
	UPROPERTY(VisibleAnywhere, Category = "State")
	bool _isAimingDownsight = false;

public:
	// Delegates
	UPROPERTY(BlueprintAssignable)
	FOnDashIsUsedSignature _onDashIsUsedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnDashIsUsedSignature _onDashIsRefilledDelegate;

// Methods
public:
	// Sets default values for this character's properties
	AMyTournamentCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Getters
	UFUNCTION(BlueprintPure, Category = "Vitals")
	UEntityVitalsComponent* GetVitalsComponent() const { return _vitalsComponent; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryComponent* GetInventoryComponent() const { return _inventoryComponent; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void CustomJump();

	UFUNCTION()
	void Landed(const FHitResult& hit) override;

	UFUNCTION()
	void Dash();

	UFUNCTION()
	virtual void PerformDash();

	UFUNCTION()
	void RefillOneDash();

	UFUNCTION()
	void DetectRunnableWalls();

	UFUNCTION()
	void StartWallRun(EWallRunModes wRunMode, AActor* curWall);

	UFUNCTION()
	void HandleWallRunMovements(float deltaTime);

	UFUNCTION()
	void EndWallRun();

	UFUNCTION()
	bool IsInNormalGroundedMovements();

	UFUNCTION()
	void CustomCrouchToggle();

	// IWeaponOperator
	UFUNCTION()
	virtual FVector GetAimPoint_Implementation() override;

	UFUNCTION()
	bool IsAimingDownsight_Implementation() override;

	UFUNCTION()
	bool CanPrimaryFire_Implementation() override;

	UFUNCTION()
	bool CanSecondaryFire_Implementation() override;

	// BP Functions

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnWeaponFiresPrimary();

	UFUNCTION(BlueprintPure)
	bool BPF_IsWallRunning();

	UFUNCTION(BlueprintPure)
	bool BPF_IsWallRunningRight();

	UFUNCTION(BlueprintPure)
	int BPF_GetCurrentWallRunningMode();

	UFUNCTION(BlueprintCallable)
	bool BPF_ActivateCurrentWeapon();

	UFUNCTION(BlueprintPure)
	virtual FRotator BPF_GetWorldRotation();


	// Pure Functions that needs to be overriden by child classes
	// 
	// Inputs Checks that can be different from player and bots 
	// For example, in order to dash the player has to have a certain movementVector, while it's not necessary for the AI
	UFUNCTION()
	virtual bool CanDash() PURE_VIRTUAL(AMyTournamentCharacterBase::CanDash, return false;);

	UFUNCTION()
	virtual void HandleOnWeaponFirePrimary() PURE_VIRTUAL(AMyTournamentCharacterBase::HandleOnWeaponFirePrimary, ;);
	UFUNCTION()
	virtual void HandleOnWeaponFireSecondary() PURE_VIRTUAL(AMyTournamentCharacterBase::HandleOnWeaponFireSecondary, ;);

	UFUNCTION()
	virtual void AimDownsight() PURE_VIRTUAL(AMyTournamentCharacterBase::AimDownsight, ;);
	UFUNCTION()
	virtual void StopAimingDownsight() PURE_VIRTUAL(AMyTournamentCharacterBase::StopAimingDownsight, ;);

	UFUNCTION()
	virtual void HandleOnWeaponEquipped(const FWeaponInInventoryEntry& weaponEntry) PURE_VIRTUAL(AMyTournamentCharacterBase::HandleOnWeaponEquipped, ;);
	UFUNCTION()
	virtual void HandleOnWeaponUnequipped(const FWeaponInInventoryEntry& weaponEntry) PURE_VIRTUAL(AMyTournamentCharacterBase::HandleOnWeaponUnequipped, ;);
};
