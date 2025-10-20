// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../Player/MyTournamentCharacter.h"
#include "Components/SphereComponent.h"
#include "PickupBase.generated.h"

class UItemAsset;

UCLASS(BlueprintType, Blueprintable)
class MYTOURNAMENT_API APickupBase : public AActor
{
	GENERATED_BODY()
	
// Properties
private:
	UPROPERTY(EditInstanceOnly, Category = "Pickup | Base")
	TSoftObjectPtr<UItemAsset> _itemAsset;

	UPROPERTY(EditAnywhere, Category = "Pickup | Base")
	TObjectPtr<UStaticMeshComponent> _mesh; // will have some sort of collider


public:	
	// Sets default values for this actor's properties
	APickupBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Initialize();

	UFUNCTION()
	void OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
