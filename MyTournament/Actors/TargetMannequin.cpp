// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetMannequin.h"
#include "Components/EntityVitalsComponent.h"

// Sets default values
ATargetMannequin::ATargetMannequin()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_skeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	check(_skeletalMesh != nullptr);

	SetRootComponent(_skeletalMesh);

	_weaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	check(_weaponMesh != nullptr);

	_weaponMesh->SetupAttachment(_skeletalMesh, FName(TEXT("hand_r")));

	_entityVitals = CreateDefaultSubobject<UEntityVitalsComponent>(TEXT("Vitals"));
	check(_entityVitals != nullptr);
}

// Called when the game starts or when spawned
void ATargetMannequin::BeginPlay()
{
	Super::BeginPlay();
	
	_entityVitals->CustomInitialize();
	_entityVitals->_onVitalsChangeDelegate.AddUniqueDynamic(this, &ATargetMannequin::HandleOnVitalsChange);
	_entityVitals->_onDeathDelegate.AddUniqueDynamic(this, &ATargetMannequin::HandleOnDeath);

	_skeletalMesh->SetAnimInstanceClass(_defaultAnimBlueprint->GeneratedClass);
	_animInstance = _skeletalMesh->GetAnimInstance();
}

// Called every frame
void ATargetMannequin::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void ATargetMannequin::HandleOnVitalsChange(float newHP, float newAP)
{
	// Trigger the BP_ implemented function
	BP_OnVitalsChange(newHP, newAP);
}

void ATargetMannequin::HandleOnDeath()
{
	_skeletalMesh->SetCollisionProfileName(FName("DeadCharacter"), true);
	_skeletalMesh->SetSimulatePhysics(true);
}
