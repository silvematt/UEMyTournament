// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTournamentCharacterBase.h"
#include "UI/MyTournamentUI.h"
#include "Animation/MyTournamentAnimInstance.h"
#include "Actors/Components/EntityVitalsComponent.h"
#include "Actors/Components/InventoryComponent.h"
#include "Actors/Items/WeaponInstance.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AMyTournamentCharacterBase::AMyTournamentCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Add Components
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation; // flag the 3rd person mesh as representation, so it only casts shadow for this camera

	// Create the Vitals component
	_vitalsComponent = CreateDefaultSubobject<UEntityVitalsComponent>(TEXT("VitalsComponent"));
	check(_vitalsComponent != nullptr);

	// Create the inventory component
	_inventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	check(_inventoryComponent != nullptr);
}

// Called when the game starts or when spawned
void AMyTournamentCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	GetMesh()->SetAnimInstanceClass(_tpsDefaultAnim->GeneratedClass);
	_characterAnimInstance = Cast<UMyTournamentAnimInstance>(GetMesh()->GetAnimInstance());

	// Get the movement component
	_movementComponent = Cast<UCharacterMovementComponent>(GetMovementComponent());

	// Bind delegates and input is all done in the derived classes

	GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("MyTournamentCharacter initialized!"));
}

// Called every frame
void AMyTournamentCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If we're falling, stop crouching
	if (IsCrouched() && _movementComponent->IsFalling())
		UnCrouch();

	// Checks for wall run
	if(_movementComponent->IsFalling() && !_wallRunIsWallRunning)
		DetectRunnableWalls();

	if (_wallRunIsWallRunning)
		HandleWallRunMovements(DeltaTime);
}

void AMyTournamentCharacterBase::CustomJump()
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

void AMyTournamentCharacterBase::Landed(const FHitResult& hit)
{
	Super::Landed(hit);

	// Reset last wall run pointer, so that we can wallrun again
	_wallRunLastWall = nullptr;
}

void AMyTournamentCharacterBase::PerformDash()
{
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
		GetWorldTimerManager().SetTimer(tHandle, this, &AMyTournamentCharacterBase::RefillOneDash, _dashRestoreCooldown, false);

		// Broadcast the event
		_onDashIsUsedDelegate.Broadcast(_dashCurAvaiable);

		// Update animator
		if (_movementVector.Y >= 0.7f)
		{
			_characterAnimInstance->_bIsDashing = true;
		}
	}
}

void AMyTournamentCharacterBase::Dash()
{
	// Use the dash button to uncrouch if crouched
	if (IsCrouched())
		UnCrouch();
	else
	{
		// If there's at least one dash available to the player and he's moving over the _dashInputThreshold
		if (CanDash())
		{
			// We can perform a dash
			PerformDash();
		}
	}
}

void AMyTournamentCharacterBase::RefillOneDash()
{
	if (_dashCurAvaiable < _dashAvailableNum)
		_dashCurAvaiable++;
	else
		_dashCurAvaiable = _dashAvailableNum;

	_onDashIsRefilledDelegate.Broadcast(_dashCurAvaiable);
}

bool AMyTournamentCharacterBase::IsInNormalGroundedMovements()
{
	return !_wallRunIsWallRunning;
}

void AMyTournamentCharacterBase::DetectRunnableWalls()
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
		FVector start = GetActorLocation();
		FVector end = start + (GetActorRightVector() * _wallLateralRunCheckVectorLength);

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
			end = start + (-GetActorRightVector() * _wallLateralRunCheckVectorLength);

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
			end = start + (GetActorForwardVector() * _wallVerticalRunCheckVectorLength);

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

void AMyTournamentCharacterBase::StartWallRun(EWallRunModes wRunMode, AActor* curWall)
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

void AMyTournamentCharacterBase::HandleWallRunMovements(float deltaTime)
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
		{
			// Launch the character to climb the wall we've run on
			if (!FMath::IsNearlyEqual(_verticalWallRunLaunchForceToClimb, 0.0f, 0.1f))
			{
				LaunchCharacter(FVector(0, 0, _verticalWallRunLaunchForceToClimb), true, true);
			}

			EndWallRun();
		}
	}
}

void AMyTournamentCharacterBase::EndWallRun()
{
	_wallRunIsWallRunning = false;
	_movementComponent->GravityScale = 1.0f;
	_wallRunTimeWallRunning = 0.0f;
}

void AMyTournamentCharacterBase::CustomCrouchToggle()
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

// Returns the point hit by the center of the camera
FVector AMyTournamentCharacterBase::GetAimPoint_Implementation()
{
	// The target position to return
	FVector targetPosition(0,0,0);

	return targetPosition;
}

// Returns if the player can fire or not
bool AMyTournamentCharacterBase::CanPrimaryFire_Implementation()
{
	// Player cannot fire if he's vertical wallrunning
	return (!_wallRunIsWallRunning || _curWallRunMode != EWallRunModes::VerticalWall);
}

// Returns if the player can fire or not
bool AMyTournamentCharacterBase::CanSecondaryFire_Implementation()
{
	// For now follow same behavior of CanPrimaryFire
	return CanPrimaryFire_Implementation();
}

bool AMyTournamentCharacterBase::IsAimingDownsight_Implementation()
{
	return _isAimingDownsight;
}

// Blueprints
bool AMyTournamentCharacterBase::BPF_IsWallRunning()
{
	return _wallRunIsWallRunning;
}

bool AMyTournamentCharacterBase::BPF_IsWallRunningRight()
{
	return _curWallRunMode == EWallRunModes::RightWall;
}

bool AMyTournamentCharacterBase::BPF_ActivateCurrentWeapon()
{
	if (auto weap = _inventoryComponent->GetCurrentWeaponInstance())
	{
		weap->SetWeaponActivated(true);
		return true;
	}

	return false;
}

int AMyTournamentCharacterBase::BPF_GetCurrentWallRunningMode()
{
	return static_cast<int32>(_curWallRunMode);
}


FRotator AMyTournamentCharacterBase::BPF_GetWorldRotation()
{
	FRotator rot(100, 100, 100); // temp, bot will use the world rotation to aim at its target
	return rot;
}
