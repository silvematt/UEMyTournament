// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "../Interfaces/WeaponOperator.h"
#include "MyTournamentCharacter.generated.h"

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

UCLASS()
class MYTOURNAMENT_API AMyTournamentCharacter : public ACharacter, public IWeaponOperator
{
	GENERATED_BODY()

// Properties
protected:

	// Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> _baseInputMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> _moveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> _jumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> _lookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> _crouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> _firePrimaryAction;

	UPROPERTY(VisibleAnywhere, Category = "Input")
	FVector2D _movementVector;

	// Dash
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "	Dash | Input")
	TObjectPtr<UInputAction> _dashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash | Behavior")
	float _dashInputThreshold = 0.25f; // _movementVector.Length() has to be >= than _dashInputThreshold for the dash to be able to happen

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash | Behavior")
	float _dashForce = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash | Behavior")
	float _dashVerticalLift = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash | Behavior")
	float _dashGroundedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash | Behavior")
	float _dashAirboneMultiplier = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash | Behavior")
	int _dashAvailableNum = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash | Behavior")
	float _dashRestoreCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash | State")
	int _dashCurAvaiable = 2;

	// Components

	// - Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> _cameraComponent;

	UPROPERTY(EditAnywhere, Category = Camera)
	FVector _camerPositionOffset = FVector(2.8f, 5.9f, 0.0f);

	UPROPERTY(EditAnywhere, Category = Camera)
	float _cameraFOV = 90.0f;

	UPROPERTY(EditAnywhere, Category = Camera)
	float _cameraScale = 0.6f;

	// - Camera Holder
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USceneComponent> _cameraHolder;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	FTransform _fpsMeshHeadSocket; // to set _cameraHolder position always to be on the head socket

	// - Animation
	UPROPERTY(VisibleAnywhere, Category = "Animation")
	TObjectPtr<UMyTournamentAnimInstance> _characterAnimInstance;

	UPROPERTY(VisibleAnywhere, Category = "Animation")
	TObjectPtr<UMyTournamentAnimInstance> _fpsAnimInstance;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimBlueprint> _fpsDefaultAnim;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimBlueprint> _tpsDefaultAnim;

	// - Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "First Person")
	TObjectPtr<USkeletalMeshComponent> _fpsMesh;

	// - Character Component (inerithed)
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UCharacterMovementComponent> _movementComponent;

	// HUD
	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UMyTournamentUI> _myTournamentUIClass;

	TObjectPtr<UMyTournamentUI> _myTournamentUI;

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
	float _wallRunCheckVectorLength = 40.0f; // lenght of the vector to check for walls on the left and right of the player

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Settings")
	float _wallRunStickCheckVectorLength = 80.0f;	// Once the wall run has started, this is the lenght of the vector that checks if we're still attached to the wall
													// it's bigger than _wallRunCheckVectorLength because 1: we want _wallRunCheckVectorLength to be small for precise input, 
													// 2: if the "stick-check" has the vector length too small, if we're sliding along a rounded surface it may fail

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Settings")
	float _wallRunCameraTiltValue = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Settings")
	float _wallRunCameraTiltInterpSpeed = 5.0f;

	// - Behavior
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Behavior")
	float _wallRunGravityScaleModifier = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Behavior")
	float _wallRunSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WallRun | Settings")
	float _wallRunForwardObstacleCheckLength = 50.0f;
	

	// - State
	UPROPERTY(VisibleAnywhere, Category = "WallRun | State")
	bool _wallRunIsWallRunning = false;

	UPROPERTY(VisibleAnywhere, Category = "WallRun | State")
	bool _wallRunIsWallRunningRight = false;

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

public:
	// Delegates
	UPROPERTY(BlueprintAssignable)
	FOnDashIsUsedSignature _onDashIsUsedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnDashIsUsedSignature _onDashIsRefilledDelegate;

// Methods
public:
	// Sets default values for this character's properties
	AMyTournamentCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void Move(const FInputActionValue& inputValue);

	UFUNCTION()
	void Look(const FInputActionValue& inputValue);

	UFUNCTION()
	void StopMoving();

	UFUNCTION()
	void CustomJump();

	UFUNCTION()
	void Landed(const FHitResult& hit) override;

	UFUNCTION()
	void Dash();

	UFUNCTION()
	void RefillOneDash();

	UFUNCTION()
	void DetectRunnableWalls();

	UFUNCTION()
	void StartWallRun(bool isRight, AActor* curWall);

	UFUNCTION()
	void HandleWallRunMovements(float deltaTime);

	UFUNCTION()
	void EndWallRun();

	UFUNCTION()
	void UpdateCameraTilt(float deltaTime);

	UFUNCTION()
	bool IsInNormalGroundedMovements();

	UFUNCTION()
	void CustomCrouchToggle();

	UFUNCTION()
	void OnWeaponIsEquipped(const FWeaponInInventoryEntry& weaponEntry);

	UFUNCTION()
	void OnWeaponIsUnequipped(const FWeaponInInventoryEntry& weaponEntry);

	// IWeaponOperator
	UFUNCTION()
	FVector GetAimPoint_Implementation() override;

	// Blueprint
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool BPF_IsWallRunning();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool BPF_IsWallRunningRight();

	// Getters
	UFUNCTION(BlueprintPure, Category = "Vitals")
	UEntityVitalsComponent* GetVitalsComponent() const { return _vitalsComponent; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryComponent* GetInventoryComponent() const { return _inventoryComponent; }
};

