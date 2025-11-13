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

	UPROPERTY(BlueprintReadOnly, Category = "Projectile | Status")
	TSubclassOf<AActor> _impactVFX;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Projectile | Status")
	bool _shot = false; // if shot is true, then we check for collision. If shot is false this projectile is still initializing

	UPROPERTY(EditAnywhere, Category = "Projectile | Settings")
	float _travelSpeed = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile | Settings")
	float _lifespan = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile | Explosion | Settings")
	bool _isAnExplosiveBullet = false;

	UPROPERTY(EditAnywhere, Category = "Projectile | Explosion | Settings", meta = (EditCondition = "_isAnExplosiveBullet == true", EditConditionHides))
	float _explosionRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile | Explosion | Settings", meta = (EditCondition = "_isAnExplosiveBullet == true", EditConditionHides))
	float _explosionForce = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile | Explosion | Settings", meta = (EditCondition = "_isAnExplosiveBullet == true", EditConditionHides))
	bool _launchCharactersOnExplosionHit = true;

	UPROPERTY(EditAnywhere, Category = "Projectile | Explosion | Settings", meta = (EditCondition = "_isAnExplosiveBullet == true", EditConditionHides))
	float _explosionLaunchHorizontalForce = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile | Explosion | Settings", meta = (EditCondition = "_isAnExplosiveBullet == true", EditConditionHides))
	float _explosionLaunchVerticalForce = 300.0f;


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

	//UFUNCTION()
	//void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	// Explosion are managed with OnHit
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void ExplosionCheck(const FVector& center);
};
