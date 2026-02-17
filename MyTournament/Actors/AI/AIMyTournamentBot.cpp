// Fill out your copyright notice in the Description page of Project Settings.


#include "AIMyTournamentBot.h"
#include "AIControllerMyTournamentBot.h"
#include "Components/CapsuleComponent.h"
#include "Actors/Components/InventoryComponent.h"
#include "Actors/Items/WeaponInstance.h"
#include "Animation/MyTournamentAnimInstance.h"
#include "BrainComponent.h"

// Sets default values
AAIMyTournamentBot::AAIMyTournamentBot()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAIMyTournamentBot::BeginPlay()
{
	Super::BeginPlay();
	
	_myController = Cast<AAIControllerMyTournamentBot>(GetController());

	if (!_myController)
		UE_LOG(LogTemp, Error, TEXT("AAIMyTournamentBot: _myController is null!"));

	// Init Anim
	GetMesh()->SetAnimInstanceClass(_tpsDefaultAnim->GeneratedClass);
	_characterAnimInstance = Cast<UMyTournamentAnimInstance>(GetMesh()->GetAnimInstance());

	// Initialize Vitals
	_vitalsComponent->CustomInitialize();
	_vitalsComponent->_onVitalsChangeDelegate.AddUniqueDynamic(this, &AAIMyTournamentBot::HandleOnVitalsChange);
	_vitalsComponent->_onDeathDelegate.AddUniqueDynamic(this, &AAIMyTournamentBot::HandleOnDeath);

	// Set OnEquipped/Unequipped callbacks
	_inventoryComponent->_onWeaponIsEquippedDelegate.AddUniqueDynamic(this, &AAIMyTournamentBot::HandleOnWeaponEquipped);
	_inventoryComponent->_onWeaponIsUnequippedDelegate.AddUniqueDynamic(this, &AAIMyTournamentBot::HandleOnWeaponUnequipped);

	/// Initialize Inventory
	_inventoryComponent->CustomInitialize(this);
}

// Called every frame
void AAIMyTournamentBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAIMyTournamentBot::HandleOnVitalsChange(float newHP, float newAP)
{
	// Trigger the BP_ implemented function
	BP_OnVitalsChange(newHP, newAP);
}

void AAIMyTournamentBot::HandleOnDeath()
{
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("DeadCharacter"), true);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (!_myController)
		_myController = Cast<AAIControllerMyTournamentBot>(GetController());

	_myController->OnControlledCharacterDeath();
}

void AAIMyTournamentBot::HandleOnWeaponEquipped(const FWeaponInInventoryEntry& weaponEntry)
{
	// For AI additional mesh is not necessary, so hide it
	weaponEntry._instance->_additionalSkeletalMesh->SetVisibility(false, true);

	// Attach the weapon to the AI's hands
	FAttachmentTransformRules attachmentRules(EAttachmentRule::SnapToTarget, true);
	weaponEntry._instance->AttachToComponent(GetMesh(), attachmentRules, FName(TEXT("HandGrip_R")));
	weaponEntry._instance->_skeletalMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	weaponEntry._instance->_skeletalMesh->SetCastShadow(true);

	// Subscribe to delegates
	//weaponEntry._instance->_onWeaponFiresPrimaryDelegate.AddUniqueDynamic(this, &AMyTournamentCharacter::HandleOnWeaponFirePrimary);
	//weaponEntry._instance->_onWeaponFiresSecondaryDelegate.AddUniqueDynamic(this, &AMyTournamentCharacter::HandleOnWeaponFireSecondary);

	// Update Anims
	// Reset current anim instance before setting a new one
	_characterAnimInstance->ResetProperties();
	GetMesh()->SetAnimInstanceClass(weaponEntry._asset->_tpsAnimBlueprint->GeneratedClass);
	_characterAnimInstance = Cast<UMyTournamentAnimInstance>(GetMesh()->GetAnimInstance());
}

void AAIMyTournamentBot::HandleOnWeaponUnequipped(const FWeaponInInventoryEntry& weaponEntry)
{

}

bool AAIMyTournamentBot::CanDash()
{
	return _dashCurAvaiable >= 1;
}
