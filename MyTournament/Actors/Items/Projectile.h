// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class AWeaponInstance;
class UProjectileMovementComponent;
class USphereComponent;


UCLASS()
class MYTOURNAMENT_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

// Properties
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile | Components")
	TObjectPtr<UStaticMeshComponent> _staticMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile | Components")
	TObjectPtr<UProjectileMovementComponent> _movementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile | Components")
	TObjectPtr<USphereComponent> _collisionComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Projectile | Status")
	TObjectPtr<AActor> _actorThatShot;

	UPROPERTY(BlueprintReadOnly, Category = "Projectile | Status")
	TObjectPtr<AWeaponInstance> _weaponThatShot;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Projectile | Status")
	bool _shot = false; // if shot is true, then we check for collision. If shot is false this projectile is still initializing

	UPROPERTY(EditAnywhere, Category = "Projectile | Settings")
	float _travelSpeed = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile | Settings")
	float _lifespan = 10.0f;

// Methods
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Must be called as soon as the projectile is instantiated
	UFUNCTION()
	void InitializeProjectile(AActor* actorThatShot, AWeaponInstance* weaponThatShot);

	// Called when the projectile collides with an object
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
