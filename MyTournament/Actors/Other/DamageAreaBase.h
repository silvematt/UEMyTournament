// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageAreaBase.generated.h"

class UShapeComponent;

/*
Damage Areas needs a blueprint derived class that Adds the Collision Component (Box, Sphere, etc)
Blueprint's BeginPlay needs to set _colliderComponent
*/
UCLASS(Abstract, Blueprintable)
class MYTOURNAMENT_API ADamageAreaBase : public AActor
{
	GENERATED_BODY()
	
// Properties
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Damage Area")
	TObjectPtr<UShapeComponent> _colliderComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Area")
	float _damagePerTick = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Area")
	float _tickRate = 1.0f; // _damagePerTick value applied every _tickRate seconds

	FTimerHandle _timerHandle;
	TArray<AActor*> _actorsIn;

// Methods
public:	
	// Sets default values for this actor's properties
	ADamageAreaBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void DamageTick();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
