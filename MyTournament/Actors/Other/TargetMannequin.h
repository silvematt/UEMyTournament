// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TargetMannequin.generated.h"

class UEntityVitalsComponent;

UCLASS()
class MYTOURNAMENT_API ATargetMannequin : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATargetMannequin();

// Properties
protected:

	// Skeletal Mesh of the mannequin
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> _skeletalMesh;

	// Skeletal Mesh of the weapon that the mannequin holds
	UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMeshComponent> _weaponMesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UEntityVitalsComponent> _entityVitals;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimBlueprint> _defaultAnimBlueprint;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAnimInstance> _animInstance;

// Methods
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called by _entityVitals's delegate onVitalsChange
	UFUNCTION()
	void HandleOnVitalsChange(float newHP, float newAP);

	// Called by _entityVitals's delegate onDeathDelegate
	UFUNCTION()
	void HandleOnDeath();

	// Allows to have a BP_ function to run when HandleOnVitalsChange gets called by _entityVitals's delegate onVitalsChange
	UFUNCTION(BlueprintImplementableEvent, Category = "Vitals")
	void BP_OnVitalsChange(float NewHP, float NewAP);
};
