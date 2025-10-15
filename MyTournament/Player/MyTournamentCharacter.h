// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "MyTournamentCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UInputComponent;
class UAnimBlueprint;
class UMyTournamentUI;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDashIsUsedSignature, int, dashesNowAvailable);

UCLASS()
class MYTOURNAMENT_API AMyTournamentCharacter : public ACharacter
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

	// - Dash
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> _dashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	float _dashForce = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	float _dashVerticalLift = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	float _dashGroundedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	float _dashAirboneMultiplier = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	int _dashAvailableNum = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	float _dashRestoreCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	int _dashCurAvaiable = 2;

	UPROPERTY(VisibleDefaultsOnly, Category = "Input")
	FVector2D _movementVector;

	// Components

	// - Camera
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> _cameraComponent;

	UPROPERTY(EditAnywhere, Category = Camera)
	FVector _camerPositionOffset = FVector(2.8f, 5.9f, 0.0f);

	UPROPERTY(EditAnywhere, Category = Camera)
	float _cameraFOV = 90.0f;

	UPROPERTY(EditAnywhere, Category = Camera)
	float _cameraScale = 0.6f;

	// - Mesh
	UPROPERTY(VisibleAnywhere, Category = "First Person")
	TObjectPtr<USkeletalMeshComponent> _fpsMesh;

	// Animation
	UPROPERTY(EditAnywhere, Category = Animation)
	TObjectPtr<UAnimBlueprint> _fpsDefaultAnim;

	UPROPERTY(EditAnywhere, Category = Animation)
	TObjectPtr<UAnimBlueprint> _tpsDefaultAnim;

	// HUD
	UPROPERTY(EditAnywhere, Category = "HUD")
	TSubclassOf<UMyTournamentUI> _myTournamentUIClass;

	TObjectPtr<UMyTournamentUI> _myTournamentUI;

public:
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
	void Dash();

	UFUNCTION()
	void RefillOneDash();
};

