// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTournamentCharacter.h"
#include "../UI/MyTournamentUI.h"
#include "Animation/MyTournamentAnimInstance.h"
#include "../Actors/Components/EntityVitalsComponent.h"
#include "../Actors/Components/InventoryComponent.h"
#include "../Actors/Items/WeaponInstance.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AMyTournamentCharacter::AMyTournamentCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Add Components
	
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

	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation; // flag the 3rd person mesh as representation, so it only casts shadow for this camera

	// Attach camera to mesh
	_cameraHolder->SetupAttachment(_fpsMesh);
	_cameraComponent->SetupAttachment(_cameraHolder);

	// Camera rotation is handled manually
	_cameraComponent->bUsePawnControlRotation = false;

	// Create the Vitals component
	_vitalsComponent = CreateDefaultSubobject<UEntityVitalsComponent>(TEXT("VitalsComponent"));
	check(_vitalsComponent != nullptr);

	// Create the inventory component
	_inventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	check(_inventoryComponent != nullptr);
}

// Called when the game starts or when spawned
void AMyTournamentCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

	GetMesh()->SetAnimInstanceClass(_tpsDefaultAnim->GeneratedClass);
	_characterAnimInstance = Cast<UMyTournamentAnimInstance>(GetMesh()->GetAnimInstance());

	// Get the movement component
	_movementComponent = Cast<UCharacterMovementComponent>(GetMovementComponent());

	// Begin HUD
	_myTournamentUI = CreateWidget<UMyTournamentUI>(playerController, _myTournamentUIClass);
	_myTournamentUI->AddToViewport(0);
	_myTournamentUI->SetOwningPlayer(playerController);

	// Set weapon callback
	_inventoryComponent->_onWeaponIsEquippedDelegate.AddUniqueDynamic(this, &AMyTournamentCharacter::HandleOnWeaponEquipped);
	_inventoryComponent->_onWeaponIsUnequippedDelegate.AddUniqueDynamic(this, &AMyTournamentCharacter::HandleOnWeaponUnequipped);

	// Custom Initialize components
	// Begin Play order can vary, so we initialize sub-components here. This ensures deterministic initialization 
	_vitalsComponent->CustomInitialize();
	_myTournamentUI->CustomInitialize();
	_inventoryComponent->CustomInitialize(this);
	_inventoryComponent->BindWeaponSwitchActions();

	GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("MyTournamentCharacter initialized!"));
}

// Called to bind functionality to input
void AMyTournamentCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* enhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		enhancedInput->BindAction(_moveAction, ETriggerEvent::Triggered, this, &AMyTournamentCharacter::Move);
		enhancedInput->BindAction(_moveAction, ETriggerEvent::Completed, this, &AMyTournamentCharacter::StopMoving);

		enhancedInput->BindAction(_jumpAction, ETriggerEvent::Started, this, &AMyTournamentCharacter::CustomJump);
		enhancedInput->BindAction(_jumpAction, ETriggerEvent::Completed, this, &AMyTournamentCharacter::StopJumping);

		enhancedInput->BindAction(_lookAction, ETriggerEvent::Triggered, this, &AMyTournamentCharacter::Look);

		enhancedInput->BindAction(_dashAction, ETriggerEvent::Started, this, &AMyTournamentCharacter::Dash);

		enhancedInput->BindAction(_crouchAction, ETriggerEvent::Started, this, &AMyTournamentCharacter::CustomCrouchToggle);
	}
}

// Called every frame
void AMyTournamentCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update _cameraHolder position to always follow the fpsMesh's head
	_fpsMeshHeadSocket = _fpsMesh->GetSocketTransform(TEXT("head"), RTS_World);
	_cameraHolder->SetWorldLocation(_fpsMeshHeadSocket.GetLocation());

	// If we're falling, stop crouching
	if (IsCrouched() && _movementComponent->IsFalling())
		UnCrouch();

	// Checks for wall run
	if(_movementComponent->IsFalling() && !_wallRunIsWallRunning)
		DetectRunnableWalls();

	if (_wallRunIsWallRunning)
		HandleWallRunMovements(DeltaTime);

	// Rotates the camera holder if wallrunning
	UpdateCameraTilt(DeltaTime);
}

void AMyTournamentCharacter::Move(const FInputActionValue& inputValue)
{
	_movementVector = inputValue.Get<FVector2D>();

	if (Controller && IsInNormalGroundedMovements())
	{
		AddMovementInput(GetActorRightVector() * _movementVector.X);
		AddMovementInput(GetActorForwardVector() * _movementVector.Y);
	}
}

void AMyTournamentCharacter::StopMoving()
{
	_movementVector = FVector2D::Zero();
}

void AMyTournamentCharacter::Look(const FInputActionValue& inputValue)
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

void AMyTournamentCharacter::CustomJump()
{
	// Use the jump button to uncrouch if crouched
	if (IsCrouched())
		UnCrouch();
	else
	{
		if (_wallRunIsWallRunning)
			EndWallRun();

		Jump();
	}
}

void AMyTournamentCharacter::Landed(const FHitResult& hit)
{
	Super::Landed(hit);

	// Reset last wall run pointer, so that we can wallrun again
	_wallRunLastWall = nullptr;
}

void AMyTournamentCharacter::Dash()
{
	// Use the dash button to uncrouch if crouched
	if (IsCrouched())
		UnCrouch();
	else
	{
		// If there's at least one dash available to the player and he's moving over the _dashInputThreshold
		if (_dashCurAvaiable >= 1 && _movementVector.Length() >= _dashInputThreshold)
		{
			// We can perform a dash
			// Get direction to dash towards to
			FVector dashVector = FVector(0, 0, 0);
			if (Controller)
			{
				// If player is wallruning, end wall run
				if (_wallRunIsWallRunning)
					EndWallRun();

				// Get the dash direction
				FVector vecRight = GetActorRightVector();
				FVector vecForward = GetActorForwardVector();

				dashVector = (vecRight * _movementVector.X * _dashForce) + (vecForward * _movementVector.Y * _dashForce) + (GetActorUpVector() * _dashVerticalLift);

				// Add the grounded/airbone multiplier
				if (_movementComponent->IsFalling())
					dashVector *= _dashAirboneMultiplier;
				else
					dashVector *= _dashGroundedMultiplier;

				LaunchCharacter(dashVector, true, true);
				_dashCurAvaiable--;

				// Set a timer that will call RefillOneDash on _dashRestoreCooldown
				FTimerHandle tHandle;
				GetWorldTimerManager().SetTimer(tHandle, this, &AMyTournamentCharacter::RefillOneDash, _dashRestoreCooldown, false);

				// Broadcast the event
				_onDashIsUsedDelegate.Broadcast(_dashCurAvaiable);

				// Update animator
				if (_movementVector.Y >= 0.7f)
				{
					_characterAnimInstance->_bIsDashing = true;
					_fpsAnimInstance->_bIsDashing = true;
				}
			}
		}
	}
}

void AMyTournamentCharacter::RefillOneDash()
{
	if (_dashCurAvaiable < _dashAvailableNum)
		_dashCurAvaiable++;
	else
		_dashCurAvaiable = _dashAvailableNum;

	_onDashIsRefilledDelegate.Broadcast(_dashCurAvaiable);
}

bool AMyTournamentCharacter::IsInNormalGroundedMovements()
{
	return !_wallRunIsWallRunning;
}

void AMyTournamentCharacter::DetectRunnableWalls()
{
	// Get forward speed
	FVector velocity = _movementComponent->Velocity;
	FVector forwardVector = GetActorForwardVector();
	float forwardVelocity = FVector::DotProduct(velocity, forwardVector); // Scalar forward speed

	// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Forward Velocity: ") + FString::SanitizeFloat(forwardVelocity));

	// Check should wallrun only if forward speed is high enough
	if (forwardVelocity >= _wallRunMinVelocityStartup)
	{
		// Cast traces left and right
		bool shouldWallrun = false;
		EWallRunModes shouldWallRunMode = EWallRunModes::None;

		// Check right
		FHitResult hitRes;
		FVector start = _cameraComponent->GetComponentLocation();
		FVector end = start + (_cameraComponent->GetRightVector() * _wallLateralRunCheckVectorLength);

		FCollisionQueryParams collParams;
		collParams.AddIgnoredActor(this);

		if(_wallRunDebugEnabled)
			DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 2.0f, 0, 1.0f);

		if (_movementVector.X >= _wallRunInputThreshold && GetWorld()->LineTraceSingleByChannel(hitRes, start, end, ECollisionChannel::ECC_Visibility, collParams))
		{
			AActor* hitActor = hitRes.GetActor();

			// Check if it's the right tag
			if (hitActor->ActorHasTag("WallRunnable") && (_wallRunLastWall == nullptr || (_wallRunLastWall != nullptr && _wallRunLastWall != hitActor))) // check if this wall is different from last wall, since we give the player one jump when he wallruns he could wallrun infinitely on the same wall without the second check
			{
				if (_wallRunDebugEnabled)
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Right Wall Run!"));

				shouldWallrun = true;
				shouldWallRunMode = EWallRunModes::RightWall;

				StartWallRun(shouldWallRunMode, hitActor);
				return;
			}
		}

		// Should we check left?
		if (!shouldWallrun)
		{
			hitRes.Reset(1.f, false);
			end = start + (-_cameraComponent->GetRightVector() * _wallLateralRunCheckVectorLength);

			if (_wallRunDebugEnabled)
				DrawDebugLine(GetWorld(), start, end, FColor::Magenta, false, 2.0f, 0, 1.0f);

			if (_movementVector.X <= -_wallRunInputThreshold && GetWorld()->LineTraceSingleByChannel(hitRes, start, end, ECollisionChannel::ECC_Visibility, collParams))
			{
				AActor* hitActor = hitRes.GetActor();

				// Check if it's the right tag
				if (hitActor->ActorHasTag("WallRunnable") && (_wallRunLastWall == nullptr || (_wallRunLastWall != nullptr && _wallRunLastWall != hitActor)))
				{
					if (_wallRunDebugEnabled)
						GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Left Wall Run!"));

					shouldWallrun = true;
					shouldWallRunMode = EWallRunModes::LeftWall;

					StartWallRun(shouldWallRunMode, hitActor);
					return;
				}
			}
		}

		// Check Vertical Wall
		if (!shouldWallrun)
		{
			hitRes.Reset(1.f, false);
			end = start + (_cameraComponent->GetForwardVector() * _wallVerticalRunCheckVectorLength);

			if (_wallRunDebugEnabled)
				DrawDebugLine(GetWorld(), start, end, FColor::Emerald, false, 2.0f, 0, 1.0f);

			if (_movementVector.Y >= _wallRunInputThreshold && FMath::IsNearlyEqual(_movementVector.X, 0.0f, 0.1f) && GetWorld()->LineTraceSingleByChannel(hitRes, start, end, ECollisionChannel::ECC_Visibility, collParams))
			{
				AActor* hitActor = hitRes.GetActor();

				// Check if it's the right tag
				if (hitActor->ActorHasTag("WallRunnable") && (_wallRunLastWall == nullptr || (_wallRunLastWall != nullptr && _wallRunLastWall != hitActor)))
				{
					if (_wallRunDebugEnabled)
						GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Vertical Wall Run!"));

					shouldWallrun = true;
					shouldWallRunMode = EWallRunModes::VerticalWall;

					StartWallRun(shouldWallRunMode, hitActor);
					return;
				}
			}
		}
	}
}

void AMyTournamentCharacter::StartWallRun(EWallRunModes wRunMode, AActor* curWall)
{
	// Intiialize state
	_curWallRunMode = wRunMode;
	_wallRunTimeWallRunning = 0.0f;
	_wallRunLastWall = curWall;

	// Kill Z velocity
	_movementComponent->Velocity.Z = 0.0f;

	// Set gravity modifier
	_movementComponent->GravityScale = (wRunMode == EWallRunModes::VerticalWall) ? _wallRunVerticalGravityScaleModifier : _wallRunLateralGravityScaleModifier;

	// Check if there's a jump available, otherwise give the player one
	if (JumpCurrentCount == JumpMaxCount)
		JumpCurrentCount--;

	_wallRunIsWallRunning = true;
	// Direction of the wallrun is set in HandleWallRunMovements
}

void AMyTournamentCharacter::HandleWallRunMovements(float deltaTime)
{
	_wallRunTimeWallRunning += 1 * deltaTime;

	// Check if the character hits the ground, if so, stop the wall run
	if (!_movementComponent->IsFalling())
	{
		EndWallRun();
		return;
	}

	// FORWARD Obstacle detection should only work on left and right wallrun
	// VERTICAL Obstacle detection should only work on vertical wallrun

	// Check if there's an obstacle in front that interrupts the wall run
	FHitResult obstacleHitRes;
	FVector obstacleCheckStart = GetActorLocation();
	FVector obstacleCheckEnd = obstacleCheckStart + (GetActorForwardVector() * _wallRunForwardObstacleCheckLength);

	// Override the end pos if it's a vertical wall run
	if(_curWallRunMode == EWallRunModes::VerticalWall)
		obstacleCheckEnd = obstacleCheckStart + (GetActorUpVector() * _wallRunVerticalObstacleCheckLength);

	FCollisionQueryParams obstacleCheckCollParams;
	obstacleCheckCollParams.AddIgnoredActor(this);

	if (_wallRunDebugEnabled)
		DrawDebugLine(GetWorld(), obstacleCheckStart, obstacleCheckEnd, FColor::Blue, false, 2.0f, 0, 1.0f);

	
		if (GetWorld()->LineTraceSingleByChannel(obstacleHitRes, obstacleCheckStart, obstacleCheckEnd, ECollisionChannel::ECC_Visibility, obstacleCheckCollParams))
		{
			if (_wallRunDebugEnabled)
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Hit something! ") + FString(obstacleHitRes.GetActor()->GetActorNameOrLabel()));

			EndWallRun();
			return;
		}

	// Interrupt wall run by input, if the player presses opposite direction + back
	if (_curWallRunMode == EWallRunModes::RightWall)
	{
		if (_movementVector.X <= -0.5f && _movementVector.Y <= -0.5f)
		{
			EndWallRun();
			return;
		}
	}
	else if (_curWallRunMode == EWallRunModes::LeftWall)
	{
		if (_movementVector.X >= 0.5f && _movementVector.Y <= -0.5f)
		{
			EndWallRun();
			return;
		}
	}
	else if (_curWallRunMode == EWallRunModes::VerticalWall)
	{
		if (_movementVector.Y <= -0.5f)
		{
			EndWallRun();
			return;
		}
	}

	// If we're still wallrunning

	// Slide left/righ walls
	if (_curWallRunMode == EWallRunModes::LeftWall || _curWallRunMode == EWallRunModes::RightWall)
	{
		bool wallRunIsWallRunningRight = _curWallRunMode == EWallRunModes::RightWall;

		FHitResult hitRes;
		FVector start = GetActorLocation();
		FVector end = start + (GetActorRightVector() * (wallRunIsWallRunningRight ? 1 : -1) * _wallRunStickCheckVectorLength);

		FCollisionQueryParams collParams;
		collParams.AddIgnoredActor(this);

		// Debug the trace
		if (_wallRunDebugEnabled)
			DrawDebugLine(GetWorld(), start, end, FColor::Green, false, 3.0f, 0, 1.0f);

		// Check if we're still attached to the wall (if the player looks away, this will fail, so the 'else' will correctly deatach him from the wall)
		if (GetWorld()->LineTraceSingleByChannel(hitRes, start, end, ECollisionChannel::ECC_Visibility, collParams))
		{
			// Check if it's the same wall we've started on
			if (hitRes.GetActor() == _wallRunLastWall)
			{
				// Update normal and impact point
				_wallRunCurWallNormal = hitRes.ImpactNormal;
				_wallRunCurWallImpactPoint = hitRes.ImpactPoint;

				// Draw wall normal (red)
				if (_wallRunDebugEnabled)
					DrawDebugLine(GetWorld(), _wallRunCurWallImpactPoint, _wallRunCurWallImpactPoint + _wallRunCurWallNormal * 200.0f, FColor::Red, false, 3.0f, 0, 2.0f);

				// Compute the local tangent to get the direction to run along the surface
				FVector forward = GetActorForwardVector();
				FVector tangent = (forward - _wallRunCurWallNormal * FVector::DotProduct(forward, _wallRunCurWallNormal));

				if (!tangent.IsNearlyZero(1e-4f))
				{
					tangent = tangent.GetSafeNormal();

					// Make sure the direction of the tangent is forward
					if (FVector::DotProduct(tangent, forward) < 0.0f)
						tangent *= -1.0f;

					// Draw the tangent
					if (_wallRunDebugEnabled)
						DrawDebugDirectionalArrow(GetWorld(), _wallRunCurWallImpactPoint, _wallRunCurWallImpactPoint + tangent * 200.0f, 30.0f, FColor::Cyan, false, 3, 0, 2.0f);

					// Set velocity along the tangent: this forces the player to follow the local curvature
					FVector newVel = tangent * _lateralWallRunSpeed;
					_movementComponent->Velocity.X = newVel.X;
					_movementComponent->Velocity.Y = newVel.Y;
					//_movementComponent->Velocity.Z = newVel.Z;  Keep z the same, so gravity still has effect
				}
				else
					EndWallRun();
			}
			else
				EndWallRun();
		}
		else
			EndWallRun();
	}
	else if (_curWallRunMode == EWallRunModes::VerticalWall)
	{
		FHitResult hitRes;
		FVector start = GetActorLocation();
		FVector end = start + (GetActorForwardVector() * _wallVerticalRunCheckVectorLength);

		FCollisionQueryParams collParams;
		collParams.AddIgnoredActor(this);

		// Debug the trace
		if (_wallRunDebugEnabled)
			DrawDebugLine(GetWorld(), start, end, FColor::Emerald, false, 3.0f, 0, 1.0f);

		// Check if we're still attached to the wall (if the player looks away, this will fail, so the 'else' will correctly deatach him from the wall)
		if (GetWorld()->LineTraceSingleByChannel(hitRes, start, end, ECollisionChannel::ECC_Visibility, collParams))
		{
			// Check if it's the same wall we've started on
			if (hitRes.GetActor() == _wallRunLastWall)
			{
				// Update normal and impact point
				_wallRunCurWallNormal = hitRes.ImpactNormal;
				_wallRunCurWallImpactPoint = hitRes.ImpactPoint;

				// Draw wall normal
				if (_wallRunDebugEnabled)
					DrawDebugLine(GetWorld(), _wallRunCurWallImpactPoint, _wallRunCurWallImpactPoint + _wallRunCurWallNormal * 200.0f, FColor::Red, false, 3.0f, 0, 2.0f);

				FVector newVel = GetActorUpVector() * _verticalWallRunSpeed;
				_movementComponent->Velocity.X = 0;
				_movementComponent->Velocity.Y = 0;
				_movementComponent->Velocity.Z = newVel.Z;
			}
			else
				EndWallRun();
		}
		else
			EndWallRun();
	}
}

void AMyTournamentCharacter::EndWallRun()
{
	_wallRunIsWallRunning = false;
	_movementComponent->GravityScale = 1.0f;
	_wallRunTimeWallRunning = 0.0f;
}

void AMyTournamentCharacter::UpdateCameraTilt(float deltaTime)
{
	if (!_wallRunIsWallRunning || _curWallRunMode == EWallRunModes::VerticalWall) // Not wallrunning? tilt back to 0
	{
		FRotator CurrentRotation = _cameraHolder->GetRelativeRotation();
		float NewRoll = FMath::FInterpTo(CurrentRotation.Pitch, 0.0f, deltaTime, _wallRunCameraTiltInterpSpeed);
		CurrentRotation.Pitch = NewRoll;
		_cameraHolder->SetRelativeRotation(CurrentRotation);
	}
	else // Wallrunning - tilt left or right
	{
		bool wallRunIsWallRunningRight = _curWallRunMode == EWallRunModes::RightWall;
		float TargetRoll = wallRunIsWallRunningRight ? -_wallRunCameraTiltValue : _wallRunCameraTiltValue;

		FRotator currentRotation = _cameraHolder->GetRelativeRotation();
		float newRoll = FMath::FInterpTo(currentRotation.Pitch, TargetRoll, deltaTime, _wallRunCameraTiltInterpSpeed);
		currentRotation.Pitch = newRoll;
		_cameraHolder->SetRelativeRotation(currentRotation);
	}
}

void AMyTournamentCharacter::CustomCrouchToggle()
{
	if (!IsCrouched() && !_movementComponent->IsFalling())
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}

void AMyTournamentCharacter::HandleOnWeaponEquipped(const FWeaponInInventoryEntry& weaponEntry)
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
	weaponEntry._instance->_onWeaponFiresPrimaryDelegate.AddUniqueDynamic(this, &AMyTournamentCharacter::HandleOnWeaponFirePrimary);
	weaponEntry._instance->_onWeaponFiresSecondaryDelegate.AddUniqueDynamic(this, &AMyTournamentCharacter::HandleOnWeaponFireSecondary);

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

void AMyTournamentCharacter::HandleOnWeaponUnequipped(const FWeaponInInventoryEntry& weaponEntry)
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
	weaponEntry._instance->_onWeaponFiresPrimaryDelegate.RemoveDynamic(this, &AMyTournamentCharacter::HandleOnWeaponFirePrimary);
	weaponEntry._instance->_onWeaponFiresSecondaryDelegate.RemoveDynamic(this, &AMyTournamentCharacter::HandleOnWeaponFireSecondary);
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

// Returns the point hit by the center of the camera
FVector AMyTournamentCharacter::GetAimPoint_Implementation()
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

		if(auto weap = _inventoryComponent->GetCurrentWeaponInstance())
			collParams.AddIgnoredActor(weap);

		World->LineTraceSingleByChannel(hitRes, traceStart, traceEnd, ECollisionChannel::ECC_Visibility, collParams);

		// Set the target position to the impact point of the hit or the end of the trace depending on whether it hit an object
		targetPosition = hitRes.bBlockingHit ? hitRes.ImpactPoint : hitRes.TraceEnd;
	}

	return targetPosition;
}

// Returns if the player can fire or not
bool AMyTournamentCharacter::CanPrimaryFire_Implementation()
{
	// Player cannot fire if he's vertical wallrunning
	return (!_wallRunIsWallRunning || _curWallRunMode != EWallRunModes::VerticalWall);
}

// Returns if the player can fire or not
bool AMyTournamentCharacter::CanSecondaryFire_Implementation()
{
	// For now follow same behavior of CanPrimaryFire
	return CanPrimaryFire_Implementation();
}

void AMyTournamentCharacter::HandleOnWeaponFirePrimary()
{
	_fpsAnimInstance->_bIsShooting = true;
	_characterAnimInstance->_bIsShooting = true;

	BP_OnWeaponFiresPrimary();
}

void AMyTournamentCharacter::HandleOnWeaponFireSecondary()
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

void AMyTournamentCharacter::AimDownsight()
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

void AMyTournamentCharacter::StopAimingDownsight()
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

bool AMyTournamentCharacter::IsAimingDownsight_Implementation()
{
	return _isAimingDownsight;
}

// Blueprints
bool AMyTournamentCharacter::BPF_IsWallRunning()
{
	return _wallRunIsWallRunning;
}

bool AMyTournamentCharacter::BPF_IsWallRunningRight()
{
	return _curWallRunMode == EWallRunModes::RightWall;
}

bool AMyTournamentCharacter::BPF_ActivateCurrentWeapon()
{
	if (auto weap = _inventoryComponent->GetCurrentWeaponInstance())
	{
		weap->SetWeaponActivated(true);
		return true;
	}

	return false;
}

int AMyTournamentCharacter::BPF_GetCurrentWallRunningMode()
{
	return static_cast<int32>(_curWallRunMode);
}
