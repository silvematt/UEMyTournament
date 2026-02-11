// Fill out your copyright notice in the Description page of Project Settings.


#include "AIMyTournamentBot.h"
#include "AIControllerMyTournamentBot.h"
#include "Components/CapsuleComponent.h"
#include "BrainComponent.h"

// Sets default values
AAIMyTournamentBot::AAIMyTournamentBot()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_entityVitals = CreateDefaultSubobject<UEntityVitalsComponent>(TEXT("Vitals"));
	check(_entityVitals != nullptr);
}

// Called when the game starts or when spawned
void AAIMyTournamentBot::BeginPlay()
{
	Super::BeginPlay();
	
	_myController = Cast<AAIControllerMyTournamentBot>(GetController());

	if (!_myController)
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("AAIMyTournamentBot: _myController is null!"));

	_entityVitals->CustomInitialize();
	_entityVitals->_onVitalsChangeDelegate.AddUniqueDynamic(this, &AAIMyTournamentBot::HandleOnVitalsChange);
	_entityVitals->_onDeathDelegate.AddUniqueDynamic(this, &AAIMyTournamentBot::HandleOnDeath);
}

// Called every frame
void AAIMyTournamentBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAIMyTournamentBot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAIMyTournamentBot::HandleOnVitalsChange(float newHP, float newAP)
{
	// Trigger the BP_ implemented function
	BP_OnVitalsChange(newHP, newAP);
}

void AAIMyTournamentBot::HandleOnDeath()
{
	GetMesh()->SetSimulatePhysics(true);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (!_myController)
		_myController = Cast<AAIControllerMyTournamentBot>(GetController());

	_myController->OnControlledCharacterDeath();
}
