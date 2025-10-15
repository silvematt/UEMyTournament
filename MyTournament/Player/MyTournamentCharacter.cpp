// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTournamentCharacter.h"
#include "../UI/MyTournamentUI.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AMyTournamentCharacter::AMyTournamentCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Add Components
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
	_cameraComponent->SetupAttachment(_fpsMesh, FName("head"));

	// Make sure that rotating the character rotates the camera
	_cameraComponent->bUsePawnControlRotation = true;
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

	GetMesh()->SetAnimInstanceClass(_tpsDefaultAnim->GeneratedClass);

	// Begin HUD
	_myTournamentUI = CreateWidget<UMyTournamentUI>(UGameplayStatics::GetPlayerController(GetWorld(), 0), _myTournamentUIClass);
	_myTournamentUI->AddToViewport(0);
	_myTournamentUI->SetOwningPlayer(playerController);

	GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("MyTournamentCharacter initialized!"));
}

// Called every frame
void AMyTournamentCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyTournamentCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* enhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		enhancedInput->BindAction(_moveAction, ETriggerEvent::Triggered, this, &AMyTournamentCharacter::Move);
		enhancedInput->BindAction(_moveAction, ETriggerEvent::Completed, this, &AMyTournamentCharacter::StopMoving);

		enhancedInput->BindAction(_jumpAction, ETriggerEvent::Started, this, &AMyTournamentCharacter::Jump);
		enhancedInput->BindAction(_jumpAction, ETriggerEvent::Completed, this, &AMyTournamentCharacter::StopJumping);

		enhancedInput->BindAction(_lookAction, ETriggerEvent::Triggered, this, &AMyTournamentCharacter::Look);

		enhancedInput->BindAction(_dashAction, ETriggerEvent::Started, this, &AMyTournamentCharacter::Dash);
	}

}

void AMyTournamentCharacter::Move(const FInputActionValue& inputValue)
{
	_movementVector = inputValue.Get<FVector2D>();

	if (Controller)
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
		AddControllerPitchInput(lookVector.Y);
	}
}

void AMyTournamentCharacter::Dash()
{
	if (_dashCurAvaiable >= 1 && _movementVector.Length() > 0.25f)
	{
		// We can perform a dash
		// Get direction to dash towards to
		FVector dashDirection = FVector(0,0,0);
		if (Controller)
		{
			FVector vecRight = GetActorRightVector();
			FVector vecForward = GetActorForwardVector();

			dashDirection = (vecRight * _movementVector.X * _dashForce) + (vecForward * _movementVector.Y * _dashForce) + (GetActorUpVector() * _dashVerticalLift);

			if (GetCharacterMovement()->IsFalling())
				dashDirection *= _dashAirboneMultiplier;
			else
				dashDirection *= _dashGroundedMultiplier;
			
			LaunchCharacter(dashDirection, true, true);

			FTimerHandle tHandle;
			GetWorldTimerManager().SetTimer(tHandle, this, &AMyTournamentCharacter::RefillOneDash, _dashRestoreCooldown, false);
			_dashCurAvaiable--;

			_onDashIsUsedDelegate.Broadcast(_dashCurAvaiable);
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