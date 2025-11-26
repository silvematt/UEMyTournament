// Fill out your copyright notice in the Description page of Project Settings.


#include "EntityVitalsComponent.h"

// Sets default values for this component's properties
UEntityVitalsComponent::UEntityVitalsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEntityVitalsComponent::BeginPlay()
{
	Super::BeginPlay();

}

// CustomInitialize is called by the owner of this component (like the player)
void UEntityVitalsComponent::CustomInitialize()
{
	// Initialize Current Health
	_currentHealth = _maxHealth / 2.0f;
	_currentArmor = _maxArmor / 2.0f;

	_isAlive = true;

	_onVitalsChange.Broadcast(_currentHealth, _currentArmor);
}

// Called every frame
void UEntityVitalsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UEntityVitalsComponent::Death()
{
	_currentHealth = 0.0f;
	_isAlive = false;

	_onDeathDelegate.Broadcast();
}

void UEntityVitalsComponent::ApplyDamage_Implementation(float dmgAmount)
{
	if (dmgAmount <= 0.0f || !_isAlive)
		return;

	const float factor = (_currentArmor >= 1.0f) ? FMath::Min(10.0f / _currentArmor, 1.0f) : 1.0f;
	float healthLoss = dmgAmount * factor;

	_currentHealth = FMath::Clamp(_currentHealth - (healthLoss), 0.0f, _maxHealth);
	_currentArmor = FMath::Clamp(_currentArmor - (dmgAmount), 0.0f, _maxArmor);

	_onVitalsChange.Broadcast(_currentHealth, _currentArmor);

	if (_currentHealth <= 0.0f)
		Death();
}

void UEntityVitalsComponent::AddHealth_Implementation(float hpAmount)
{
	_currentHealth = FMath::Clamp(_currentHealth + hpAmount, 0.0f, _maxHealth);

	_onVitalsChange.Broadcast(_currentHealth, _currentArmor);
}

void UEntityVitalsComponent::AddArmor_Implementation(float armorAmount)
{
	_currentArmor = FMath::Clamp(_currentArmor + armorAmount, 0.0f, _maxArmor);

	_onVitalsChange.Broadcast(_currentHealth, _currentArmor);
}

bool UEntityVitalsComponent::IsAlive_Implementation() const
{
	return _currentHealth >= 0.0f;
}

bool UEntityVitalsComponent::IsAtMaxHealth_Implementation() const
{
	return _currentHealth >= _maxHealth;
}

bool UEntityVitalsComponent::IsAtMaxArmor_Implementation() const
{
	return _currentArmor >= _maxArmor;
}

float UEntityVitalsComponent::GetLocationalDamageMultiplier_Implementation(FName boneName) const
{
	if (_locationalDamageMap.Contains(boneName))
		return _locationalDamageMap[boneName];

	return 1.0f;
}