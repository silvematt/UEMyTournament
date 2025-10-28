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

	// Make sure that rotating the character rotates the camera
	_cameraComponent->bUsePawnControlRotation = false;

	// Create the Vitals component
	_vitalsComponent = CreateDefaultSubobject<UEntityVitalsComponent>(TEXT("VitalsComponent"));

	// Create the inventory component
	_inventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

// Called when the game starts or when spawned
void AMyTournamentCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* playerController = Cast<APlayerController>(Controller);
	

	if (playerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* inputSubsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
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
	_myTournamentUI = CreateWidget<UMyTournamentUI>(UGameplayStatics::GetPlayerController(GetWorld(), 0), _myTournamentUIClass);
	_myTournamentUI->AddToViewport(0);
	_myTournamentUI->SetOwningPlayer(playerController);

	// Set weapon callback
	_inventoryComponent->_onWeaponIsEquippedDelegate.AddUniqueDynamic(this, &AMyTournamentCharacter::OnWeaponIsEquipped);

	// Custom Initialize components
	// Begin Play order can vary, so this ensures proper initialization of components
	_vitalsComponent->CustomInitialize();
	_myTournamentUI->CustomInitialize();
	_inventoryComponent->CustomInitialize();

	GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("MyTournamentCharacter initialized!"));
}

// Called every frame
void AMyTournamentCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update _cameraHolder position to always follow the fpsMesh's head
	_fpsMeshHeadSocket = _fpsMesh->GetSocketTransform(TEXT("head"), RTS_World);
	_cameraHolder->SetWorldLocation(_fpsMeshHeadSocket.GetLocation());

	// If we're falling, stop crouchign
	if (IsCrouched() && _movementComponent->IsFalling())
		UnCrouch();

	// Checks for wall run
	DetectRunnableWalls();

	if (_wallRunIsWallRunning)
	{
		HandleWallRunMovements(DeltaTime);
	}

	// Rotates the camera holder if wallrunning
	UpdateCameraTilt(DeltaTime);
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

	if (Controller)
	{
		AddControllerYawInput(lookVector.X);
		//AddControllerPitchInput(lookVector.Y);  bUsePawnControlRotation is false now
		
		float deltaPitch = lookVector.Y * -2.5f;

		FRotator curRot = _cameraComponent->GetRelativeRotation();
		curRot.Pitch = FMath::Clamp(FMath::UnwindDegrees(curRot.Pitch) + deltaPitch, -90.0f, 90.0f);

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

	_wallRunLastWall = nullptr;
}

void AMyTournamentCharacter::Dash()
{
	// Use the dash button to uncrouch if crouched
	if (IsCrouched())
		UnCrouch();
	else
	{
		// If there's one dash available to the player and he's moving over the _dashInputThreshold
		if (!IsCrouched() && _dashCurAvaiable >= 1 && _movementVector.Length() >= _dashInputThreshold)
		{
			// We can perform a dash
			// Get direction to dash towards to
			FVector dashDirection = FVector(0, 0, 0);
			if (Controller)
			{
				// If player is wallruning, end wall run
				if (_wallRunIsWallRunning)
					EndWallRun();

				// Get the dash direction
				FVector vecRight = GetActorRightVector();
				FVector vecForward = GetActorForwardVector();

				dashDirection = (vecRight * _movementVector.X * _dashForce) + (vecForward * _movementVector.Y * _dashForce) + (GetActorUpVector() * _dashVerticalLift);

				// Add the grounded/airbone multiplier
				if (_movementComponent->IsFalling())
					dashDirection *= _dashAirboneMultiplier;
				else
					dashDirection *= _dashGroundedMultiplier;

				LaunchCharacter(dashDirection, true, true);

				// Set a timer that will call RefillOneDash on _dashRestoreCooldown
				FTimerHandle tHandle;
				GetWorldTimerManager().SetTimer(tHandle, this, &AMyTournamentCharacter::RefillOneDash, _dashRestoreCooldown, false);
				_dashCurAvaiable--;

				// Broadcast the event
				_onDashIsUsedDelegate.Broadcast(_dashCurAvaiable);

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
	if (_movementComponent->IsFalling() && !_wallRunIsWallRunning)
	{

		// Cast traces left and right
		bool shouldWallrun = false;
		bool shouldWallrunRight = false;

		// Check right
		FHitResult hitRes;
		FVector start = _cameraComponent->GetComponentLocation();
		FVector end = start + (_cameraComponent->GetRightVector() * _wallRunCheckVectorLength);

		FCollisionQueryParams collParams;
		collParams.AddIgnoredActor(this);

		// Get forward speed
		FVector velocity = _movementComponent->Velocity;
		FVector forwardVector = GetActorForwardVector();
		float forwardVelocity = FVector::DotProduct(velocity, forwardVector); // Scalar forward speed

		// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Forward Velocity: ") + FString::SanitizeFloat(forwardVelocity));

		// Check should wallrun only if forward speed is high enough
		if (forwardVelocity >= _wallRunMinVelocityStartup)
		{
			if(_wallRunDebugEnabled)
				DrawDebugLine(GetWorld(), start, end, FColor::Red, false, 2.0f, 0, 1.0f);

			if (_movementVector.X >= _wallRunInputThreshold && GetWorld()->LineTraceSingleByChannel(hitRes, start, end, ECollisionChannel::ECC_Visibility, collParams))
			{
				AActor* hitActor = hitRes.GetActor();

				// Check if it's the right tag
				if (hitActor->ActorHasTag("WallRunnable") && (_wallRunLastWall == nullptr || (_wallRunLastWall != nullptr && _wallRunLastWall != hitActor))) // check if this wall is different from last wall, since we give the player one jump when he wallruns he could wallrun infinitely without this
				{
					if (_wallRunDebugEnabled)
						GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Right wall run!"));

					shouldWallrun = true;
					shouldWallrunRight = true;

					StartWallRun(shouldWallrunRight, hitActor);
				}
			}

			// Should we check left?
			if (!shouldWallrun)
			{
				hitRes.Reset(1.f, false);
				end = start + (-_cameraComponent->GetRightVector() * _wallRunCheckVectorLength);

				if (_wallRunDebugEnabled)
					DrawDebugLine(GetWorld(), start, end, FColor::Magenta, false, 2.0f, 0, 1.0f);

				if (_movementVector.X <= -_wallRunInputThreshold && GetWorld()->LineTraceSingleByChannel(hitRes, start, end, ECollisionChannel::ECC_Visibility, collParams))
				{
					AActor* hitActor = hitRes.GetActor();

					// Check if it's the right tag
					if (hitActor->ActorHasTag("WallRunnable") && (_wallRunLastWall == nullptr || (_wallRunLastWall != nullptr && _wallRunLastWall != hitActor)))
					{
						if (_wallRunDebugEnabled)
							GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Left wall run!"));

						shouldWallrun = true;
						shouldWallrunRight = false;

						StartWallRun(shouldWallrunRight, hitActor);
					}
				}
			}
		}
	}
}

void AMyTournamentCharacter::StartWallRun(bool isRight, AActor* curWall)
{
	// Intiialize state
	_wallRunIsWallRunningRight = isRight;
	_wallRunTimeWallRunning = 0.0f;
	_wallRunLastWall = curWall;

	// Kill Z velocity
	_movementComponent->Velocity.Z = 0.0f;

	// Set WallRun velocity to a fixed speed but keep direction the same
	FVector curVelocity = _movementComponent->Velocity;
	curVelocity = curVelocity.GetSafeNormal() * _wallRunSpeed;
	_movementComponent->Velocity = curVelocity;

	// Set gravity modifier
	_movementComponent->GravityScale = _wallRunGravityScaleModifier;

	// Check if there's a jump available, otherwise give the player one
	if (JumpCurrentCount == JumpMaxCount)
		JumpCurrentCount--;

	_wallRunIsWallRunning = true;
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

	// Check if there's an obstacle in front that interrupts the wall run
	FHitResult obstacleHitRes;
	FVector obstacleCheckStart = GetActorLocation();
	FVector obstacleCheckEnd = obstacleCheckStart + (GetActorForwardVector() * _wallRunForwardObstacleCheckLength);

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
	if (_wallRunIsWallRunningRight)
	{
		if (_movementVector.X <= -0.5f && _movementVector.Y <= -0.5f)
		{
			EndWallRun();
			return;
		}
	}
	else
	{
		if (_movementVector.X >= 0.5f && _movementVector.Y <= -0.5f)
		{
			EndWallRun();
			return;
		}
	}

	// If we're still wallrunning, follows the wall's surface tangent

	// Trace the wall, so we can stick to it
	FHitResult hitRes;
	FVector start = GetActorLocation();
	FVector end = start + (GetActorRightVector() * (_wallRunIsWallRunningRight ? 1 : -1) * _wallRunStickCheckVectorLength);

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
				FVector newVel = tangent * _wallRunSpeed;
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

void AMyTournamentCharacter::EndWallRun()
{
	_wallRunIsWallRunning = false;
	_movementComponent->GravityScale = 1.0f;
	_wallRunTimeWallRunning = 0.0f;
}

void AMyTournamentCharacter::UpdateCameraTilt(float deltaTime)
{
	if (!_wallRunIsWallRunning) // Not wallrunning? tilt back to 0
	{
		FRotator CurrentRotation = _cameraHolder->GetRelativeRotation();
		float NewRoll = FMath::FInterpTo(CurrentRotation.Pitch, 0.0f, deltaTime, _wallRunCameraTiltInterpSpeed);
		CurrentRotation.Pitch = NewRoll;
		_cameraHolder->SetRelativeRotation(CurrentRotation);
		return;
	}

	// Wallrunning - tilt left or right
	float TargetRoll = _wallRunIsWallRunningRight ? -_wallRunCameraTiltValue : _wallRunCameraTiltValue;

	FRotator CurrentRotation = _cameraHolder->GetRelativeRotation();
	float NewRoll = FMath::FInterpTo(CurrentRotation.Pitch, TargetRoll, deltaTime, _wallRunCameraTiltInterpSpeed);
	CurrentRotation.Pitch = NewRoll;
	_cameraHolder->SetRelativeRotation(CurrentRotation);
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

void AMyTournamentCharacter::OnWeaponIsEquipped(const FWeaponInInventoryEntry& weaponEntry)
{
	// Attach the weapon instance to the fpsHands component
	FAttachmentTransformRules attachmentRules(EAttachmentRule::SnapToTarget, true);
	weaponEntry._instance->AttachToComponent(_fpsMesh, attachmentRules, FName(TEXT("HandGrip_R")));
	weaponEntry._instance->_skeletalMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson; // flag the weapon instance as a FirstPerson mesh

	// Update Anims
	_fpsMesh->SetAnimInstanceClass(weaponEntry._instance->_fpsAnimBlueprint->GeneratedClass);
	GetMesh()->SetAnimInstanceClass(weaponEntry._instance->_tpsAnimBlueprint->GeneratedClass);

}

// Blueprints
bool AMyTournamentCharacter::BPF_IsWallRunning()
{
	return _wallRunIsWallRunning;
}

bool AMyTournamentCharacter::BPF_IsWallRunningRight()
{
	return _wallRunIsWallRunningRight;
}
