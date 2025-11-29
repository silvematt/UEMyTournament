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

	UPROPERTY(VisibleDefaultsOnly, Category = "Pickup | Base")
	TObjectPtr<UStaticMeshComponent> _mesh; // will have some sort of collider and determine the pickup collision

	UPROPERTY(VisibleDefaultsOnly, Category = "Pickup | Glow")
	TObjectPtr<UStaticMeshComponent> _glowMesh;

	UPROPERTY(EditAnywhere, Category = "Pickup | Glow")
	TObjectPtr<UMaterialInstance> _glowMeshMaterialAvailable;

	UPROPERTY(EditAnywhere, Category = "Pickup | Glow")
	TObjectPtr<UMaterialInstance> _glowMeshMaterialRespawning;

	UPROPERTY(EditAnywhere, Category = "Pickup | Base")
	bool _doesRespawn = true;

	UPROPERTY(EditAnywhere, Category = "Pickup | Base")
	bool _isCurrentlyActive = false;

	FTimerHandle _respawnTimeHandle;

	UPROPERTY(EditAnywhere, Category = "Pickup | Base")
	float _respawnTime = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Pickup | Behaviour")
	bool _doesRotate = true; 

	UPROPERTY(EditAnywhere, Category = "Pickup | Behaviour")
	float _rotSpeed = 1.0;

	UPROPERTY(EditAnywhere, Category = "Pickup | Behaviour")
	bool _doesHover = true;

	UPROPERTY(EditAnywhere, Category = "Pickup | Behaviour")
	float _hoverRange = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Pickup | Behaviour")
	float _hoverSpeed = 1.0f;

	// Location of the mesh at the start (for hovering)
	FVector _startMeshLocation;

	// Time that has been hovering
	float _hoverTime = 0.0f;

	// Audio
	UPROPERTY(EditAnywhere, Category = "Pickup | Audio")
	TObjectPtr<UAudioComponent> _audioComponent;

	UPROPERTY(EditAnywhere, Category = "Pickup | Audio")
	TObjectPtr<USoundBase> _pickupSound;

public:	
	// Sets default values for this actor's properties
	APickupBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


// Methods
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void Initialize();

	UFUNCTION()
	void OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void TakePickup(AActor* taker);

	UFUNCTION()
	void RespawnPickup();

	UFUNCTION()
	bool CanBeTakenBy(AActor* actor);

	UFUNCTION()
	bool IsCurrentlyActive();

#if WITH_EDITOR
	// Runs whenever a property on this object is changed in the editor
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
