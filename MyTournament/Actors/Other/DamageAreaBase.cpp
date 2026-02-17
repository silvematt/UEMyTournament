// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageAreaBase.h"
#include "Interfaces/Damageable.h"
#include "Components/ShapeComponent.h"
#include "Actors/Components/EntityVitalsComponent.h"

// Sets default values
ADamageAreaBase::ADamageAreaBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADamageAreaBase::BeginPlay()
{
	// Blueprint derived class will set _colliderComponent in BeginPlay
	Super::BeginPlay();

	if (_colliderComponent)
		GetWorldTimerManager().SetTimer(_timerHandle, this, &ADamageAreaBase::DamageTick, _tickRate, true);
}

// Called every frame
void ADamageAreaBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADamageAreaBase::DamageTick()
{
	_actorsIn.Empty();

	_colliderComponent->GetOverlappingActors(_actorsIn);

	for (AActor* curActor : _actorsIn)
	{
		if (curActor)
		{
			UEntityVitalsComponent* vitals = curActor->FindComponentByClass<UEntityVitalsComponent>();
			if(vitals)
				IDamageable::Execute_ApplyDamage(vitals, _damagePerTick);
		}
	}
}

