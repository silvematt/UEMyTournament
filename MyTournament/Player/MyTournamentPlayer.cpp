// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTournamentPlayer.h"
#include "UI/MyTournamentUI.h"
#include "Animation/MyTournamentAnimInstance.h"
#include "Actors/Components/EntityVitalsComponent.h"
#include "Actors/Components/InventoryComponent.h"
#include "Actors/Items/WeaponInstance.h"

// Constructor
AMyTournamentPlayer::AMyTournamentPlayer()
{
	// - Camera Holder
	_cameraHolder = CreateDefaultSubobject<USceneComponent>(TEXT("CameraHolder"));
	check(_cameraHolder != nullptr);

	// - Camera
	_cameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	check(_cameraComponent != nullptr);
	_cameraComponent->bEnableFirstPersonFieldOfView = true;
	_cameraComponent->SetFieldOfView(_cameraFOV);
	_cameraComponent->bEnableFirstPersonScale = true;
	_cameraComponent->SetFirstPersonScale(_cameraScale);

	// - Mesh
	_fpsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FpsMesh"));
	check(_fpsMesh != nullptr);
	_fpsMesh->SetupAttachment(GetMesh()); // Attach fpsMesh to 3rd person mesh
	_fpsMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson; // flag fpsMesh as a FirstPerson mesh

	// Attach camera to mesh
	_cameraHolder->SetupAttachment(_fpsMesh);
	_cameraComponent->SetupAttachment(_cameraHolder);

	// Camera rotation is handled manually
	_cameraComponent->bUsePawnControlRotation = false;
}

void AMyTournamentPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Initialize input
	APlayerController* playerController = Cast<APlayerController>(Controller);
	if (playerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* inputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
		{
			inputSubsystem->AddMappingContext(_baseInputMappingContext, 0);
		}
	}

	// Begin components
	_fpsMesh->SetOnlyOwnerSee(true);
	_fpsMesh->SetAnimInstanceClass(_fpsDefaultAnim->GeneratedClass);
	_fpsAnimInstance = Cast<UMyTournamentAnimInstance>(_fpsMesh->GetAnimInstance());

	// Begin HUD
	_myTournamentUI = CreateWidget<UMyTournamentUI>(playerController, _myTournamentUIClass);
	_myTournamentUI->AddToViewport(0);
	_myTournamentUI->SetOwningPlayer(playerController);
	_myTournamentUI->CustomInitialize();

	// Initialize Vitals
	_vitalsComponent->CustomInitialize();
	_vitalsComponent->_onVitalsChangeDelegate.AddUniqueDynamic(this, &AMyTournamentPlayer::HandleOnVitalsChange);
	_vitalsComponent->_onDeathDelegate.AddUniqueDynamic(this, &AMyTournamentPlayer::HandleOnDeath);

	// Set OnEquipped/Unequipped callbacks
	_inventoryComponent->_onWeaponIsEquippedDelegate.AddUniqueDynamic(this, &AMyTournamentPlayer::HandleOnWeaponEquipped);
	_inventoryComponent->_onWeaponIsUnequippedDelegate.AddUniqueDynamic(this, &AMyTournamentPlayer::HandleOnWeaponUnequipped);

	/// Initialize Inventory
	_inventoryComponent->CustomInitialize(this);
	_inventoryComponent->BindWeaponSwitchActions();
}

// Called to bind functionality to input
void AMyTournamentPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* enhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		enhancedInput->BindAction(_moveAction, ETriggerEvent::Triggered, this, &AMyTournamentPlayer::Move);
		enhancedInput->BindAction(_moveAction, ETriggerEvent::Completed, this, &AMyTournamentPlayer::StopMoving);

		enhancedInput->BindAction(_jumpAction, ETriggerEvent::Started, this, &AMyTournamentPlayer::CustomJump);
		enhancedInput->BindAction(_jumpAction, ETriggerEvent::Completed, this, &AMyTournamentPlayer::StopJumping);

		enhancedInput->BindAction(_lookAction, ETriggerEvent::Triggered, this, &AMyTournamentPlayer::Look);

		enhancedInput->BindAction(_dashAction, ETriggerEvent::Started, this, &AMyTournamentPlayer::Dash);

		enhancedInput->BindAction(_crouchAction, ETriggerEvent::Started, this, &AMyTournamentPlayer::CustomCrouchToggle);
	}
}

void AMyTournamentPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update _cameraHolder position to always follow the fpsMesh's head
	_fpsMeshHeadSocket = _fpsMesh->GetSocketTransform(TEXT("head"), RTS_World);
	_cameraHolder->SetWorldLocation(_fpsMeshHeadSocket.GetLocation());

	// Rotates the camera holder if wallrunning
	UpdateCameraTiltDuringWallrun(DeltaTime);
}


void AMyTournamentPlayer::Move(const FInputActionValue& inputValue)
{
	_movementVector = inputValue.Get<FVector2D>();

	if (Controller && IsInNormalGroundedMovements())
	{
		AddMovementInput(GetActorRightVector() * _movementVector.X);
		AddMovementInput(GetActorForwardVector() * _movementVector.Y);
	}
}

void AMyTournamentPlayer::StopMoving()
{
	_movementVector = FVector2D::Zero();
}

void AMyTournamentPlayer::Look(const FInputActionValue& inputValue)
{
	FVector2D lookVector = inputValue.Get<FVector2D>();

	float currentLookSensitivity = (_isAimingDownsight) ? _aimingDownsightLookSensitivity : _normalLookSensitivity;

	if (Controller)
	{
		AddControllerYawInput(lookVector.X * currentLookSensitivity);
		//AddControllerPitchInput(lookVector.Y * currentLookSensitivity);  bUsePawnControlRotation is false now

		float deltaPitch = lookVector.Y * -2.5f * currentLookSensitivity;

		FRotator curRot = _cameraComponent->GetRelativeRotation();
		curRot.Pitch = FMath::Clamp(FMath::UnwindDegrees(curRot.Pitch) + deltaPitch, -70.0f, 70.0f);

		_cameraComponent->SetRelativeRotation(curRot);
	}
}

void AMyTournamentPlayer::UpdateCameraTiltDuringWallrun(float deltaTime)
{
	if (!_wallRunIsWallRunning || _curWallRunMode == EWallRunModes::VerticalWall) // Not wallrunning? tilt back to 0
	{
		FRotator CurrentRotation = _cameraHolder->GetRelativeRotation();
		float NewRoll = FMath::FInterpTo(CurrentRotation.Roll, 0.0f, deltaTime, _wallRunCameraTiltInterpSpeed);
		CurrentRotation.Roll = NewRoll;
		_cameraHolder->SetRelativeRotation(CurrentRotation);
	}
	else // Wallrunning - tilt left or right
	{
		bool wallRunIsWallRunningRight = _curWallRunMode == EWallRunModes::RightWall;
		float TargetRoll = wallRunIsWallRunningRight ? -_wallRunCameraTiltValue : _wallRunCameraTiltValue;

		FRotator currentRotation = _cameraHolder->GetRelativeRotation();
		float newRoll = FMath::FInterpTo(currentRotation.Roll, TargetRoll, deltaTime, _wallRunCameraTiltInterpSpeed);
		currentRotation.Roll = newRoll;
		_cameraHolder->SetRelativeRotation(currentRotation);
	}
}

bool AMyTournamentPlayer::CanDash()
{
	return (_dashCurAvaiable >= 1 && _movementVector.Length() >= _dashInputThreshold);
}

void AMyTournamentPlayer::PerformDash()
{
	Super::PerformDash();

	// Update animator
	if (_movementVector.Y >= 0.7f)
	{
		_fpsAnimInstance->_bIsDashing = true;
	}
}

void AMyTournamentPlayer::HandleOnVitalsChange(float newHP, float newAP)
{
	// Trigger the BP_ implemented function
	BP_OnVitalsChange(newHP, newAP);
}

void AMyTournamentPlayer::HandleOnDeath()
{
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("DeadCharacter"), true);
}

void AMyTournamentPlayer::HandleOnWeaponEquipped(const FWeaponInInventoryEntry& weaponEntry)
{
	// FP
	// Attach the weapon instance to the fpsHands component
	FAttachmentTransformRules attachmentRules(EAttachmentRule::SnapToTarget, true);
	weaponEntry._instance->AttachToComponent(_fpsMesh, attachmentRules, FName(TEXT("HandGrip_R")));
	weaponEntry._instance->_skeletalMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson; // flag the weapon instance as a FirstPerson mesh
	weaponEntry._instance->_skeletalMesh->SetCastShadow(false);

	// Spawn TPS weapon on TP mesh
	weaponEntry._instance->_additionalSkeletalMesh->SetVisibility(true);
	weaponEntry._instance->_additionalSkeletalMesh->SetActive(true);
	weaponEntry._instance->_additionalSkeletalMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	weaponEntry._instance->_additionalSkeletalMesh->AttachToComponent(GetMesh(), attachmentRules, FName(TEXT("HandGrip_R")));
	weaponEntry._instance->_additionalSkeletalMesh->SetOnlyOwnerSee(true);

	// Subscribe to delegates
	weaponEntry._instance->_onWeaponFiresPrimaryDelegate.AddUniqueDynamic(this, &AMyTournamentPlayer::HandleOnWeaponFirePrimary);
	weaponEntry._instance->_onWeaponFiresSecondaryDelegate.AddUniqueDynamic(this, &AMyTournamentPlayer::HandleOnWeaponFireSecondary);

	// Update Anims
	// Reset current _fpsAnimInstance before setting a new one
	_fpsAnimInstance->ResetProperties();
	_fpsMesh->SetAnimInstanceClass(weaponEntry._asset->_fpsAnimBlueprint->GeneratedClass);
	_fpsAnimInstance = Cast<UMyTournamentAnimInstance>(_fpsMesh->GetAnimInstance());

	_characterAnimInstance->ResetProperties();
	GetMesh()->SetAnimInstanceClass(weaponEntry._asset->_tpsAnimBlueprint->GeneratedClass);
	_characterAnimInstance = Cast<UMyTournamentAnimInstance>(GetMesh()->GetAnimInstance());

	// Set aiming _aimingDownsightUW
	if (weaponEntry._asset->_aimsDownsightAsSecondaryFire && weaponEntry._asset->_aimDownsightUWClass)
	{
		APlayerController* playerController = Cast<APlayerController>(Controller);
		_currentAimingDownsightUW = CreateWidget<UUserWidget>(playerController, weaponEntry._asset->_aimDownsightUWClass);
		_currentAimingDownsightUW->SetOwningPlayer(playerController);
	}

	// Bind Input and use MappingContext
	APlayerController* playerController = Cast<APlayerController>(Controller);

	if (playerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* inputSubsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
		{
			inputSubsystem->AddMappingContext(weaponEntry._asset->_weaponMappingContext, 1);
		}
	}
	weaponEntry._instance->BindFirePrimaryAction(_firePrimaryAction);
	weaponEntry._instance->BindFireSecondaryAction(_fireSecondaryAction);
}

void AMyTournamentPlayer::HandleOnWeaponUnequipped(const FWeaponInInventoryEntry& weaponEntry)
{
	// Unbind mapping context
	APlayerController* playerController = Cast<APlayerController>(Controller);

	if (playerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* inputSubsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
		{
			inputSubsystem->RemoveMappingContext(weaponEntry._asset->_weaponMappingContext);
		}
	}

	// Unsubscribe from delegates we've previously (OnWeaponIsEquipped) subscribed
	weaponEntry._instance->_onWeaponFiresPrimaryDelegate.RemoveDynamic(this, &AMyTournamentPlayer::HandleOnWeaponFirePrimary);
	weaponEntry._instance->_onWeaponFiresSecondaryDelegate.RemoveDynamic(this, &AMyTournamentPlayer::HandleOnWeaponFireSecondary);
	weaponEntry._instance->UnbindInputActions();

	// Remove Aim downsight
	if (_currentAimingDownsightUW)
	{
		StopAimingDownsight();
	}

	if (_currentAimingDownsightUW)
	{
		_currentAimingDownsightUW->RemoveFromParent();
		_currentAimingDownsightUW->MarkAsGarbage();
	}

	_currentAimingDownsightUW = nullptr; // will be destroyed by GC (TODO handle it better)

	// Reset Animator
	_fpsMesh->SetAnimInstanceClass(_fpsDefaultAnim->GeneratedClass);
	_fpsAnimInstance = Cast<UMyTournamentAnimInstance>(_fpsMesh->GetAnimInstance());

	GetMesh()->SetAnimInstanceClass(_tpsDefaultAnim->GeneratedClass);
	_characterAnimInstance = Cast<UMyTournamentAnimInstance>(GetMesh()->GetAnimInstance());

	// Disable FP/TP weaponEntry._instance
	weaponEntry._instance->Destroy();
}

void AMyTournamentPlayer::HandleOnWeaponFirePrimary()
{
	_fpsAnimInstance->_bIsShooting = true;
	_characterAnimInstance->_bIsShooting = true;

	BP_OnWeaponFiresPrimary();
}

void AMyTournamentPlayer::HandleOnWeaponFireSecondary()
{
	if (auto curWeap = _inventoryComponent->GetCurrentWeaponAsset())
	{
		if (curWeap->_aimsDownsightAsSecondaryFire)
		{
			if (_currentAimingDownsightUW->IsInViewport())
			{
				StopAimingDownsight();
			}
			else
			{
				AimDownsight();
			}
		}
	}
}

void AMyTournamentPlayer::AimDownsight()
{
	if (auto curWeap = _inventoryComponent->GetCurrentWeaponInstance())
	{
		// Add UW from viewport and hide fps mesh
		_currentAimingDownsightUW->AddToViewport(0);
		_fpsMesh->SetVisibility(false, true);
		_cameraComponent->SetFieldOfView(curWeap->_weaponAsset->_aimDownsightFOV);
		_isAimingDownsight = true;
	}
}

void AMyTournamentPlayer::StopAimingDownsight()
{
	if (auto curWeap = _inventoryComponent->GetCurrentWeaponInstance())
	{
		// Remove UW from viewport and show fps mesh
		_currentAimingDownsightUW->RemoveFromViewport();
		_fpsMesh->SetVisibility(true, true);
		_cameraComponent->SetFieldOfView(_cameraFOV);
		_isAimingDownsight = false;
	}
}

// Returns the point hit by the center of the camera
FVector AMyTournamentPlayer::GetAimPoint_Implementation()
{
	// The target position to return
	FVector targetPosition;

	UWorld* const World = GetWorld();
	if (World != nullptr)
	{
		// The result of the line trace
		FHitResult hitRes;

		// Simulate a line trace from the character along the vector they're looking down
		const FVector traceStart = _cameraComponent->GetComponentLocation();
		const FVector traceEnd = traceStart + _cameraComponent->GetForwardVector() * 10000.0;

		FCollisionQueryParams collParams(SCENE_QUERY_STAT(AimTrace), true);
		collParams.AddIgnoredActor(this);

		if (auto weap = _inventoryComponent->GetCurrentWeaponInstance())
			collParams.AddIgnoredActor(weap);

		World->LineTraceSingleByChannel(hitRes, traceStart, traceEnd, ECollisionChannel::ECC_Visibility, collParams);

		// Set the target position to the impact point of the hit or the end of the trace depending on whether it hit an object
		targetPosition = hitRes.bBlockingHit ? hitRes.ImpactPoint : hitRes.TraceEnd;
	}

	return targetPosition;
}

FRotator AMyTournamentPlayer::BPF_GetWorldRotation()
{
	return _cameraComponent->GetComponentRotation();
}
