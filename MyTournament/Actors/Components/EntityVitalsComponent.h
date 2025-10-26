// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Interfaces/Damageable.h"
#include "EntityVitalsComponent.generated.h"

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageIsAppliedSignature, float, newHealth, float, newArmor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYTOURNAMENT_API UEntityVitalsComponent : public UActorComponent, public IDamageable
{
	GENERATED_BODY()

// Properties
private:
	UPROPERTY(VisibleAnywhere, Category = "Vitals")
	float _currentHealth = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float _maxHealth = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Vitals")
	float _currentArmor = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Vitals")
	float _maxArmor = 0.0f;

public:	
	// Sets default values for this component's properties
	UEntityVitalsComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnDamageIsAppliedSignature _onVitalsChange;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetCurrentHealth() { return _currentHealth; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetCurrentArmor() { return _currentArmor; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetMaxHealth() { return _maxHealth; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetMaxAmor() { return _maxArmor; }

	// IDamageable
	void ApplyDamage_Implementation(float dmgAmount) override;

	UFUNCTION()
	void AddHealth_Implementation(float hpAmount) override;

	UFUNCTION()
	void AddArmor_Implementation(float armorAmount) override;

	UFUNCTION()
	bool IsAlive_Implementation() const override;

	UFUNCTION()
	bool IsAtMaxHealth_Implementation() const override;

	UFUNCTION() 
	bool IsAtMaxArmor_Implementation() const override;
};
