// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupBase.h"
#include "Components/AudioComponent.h"
#include "Data/Items/ItemAsset.h"
#include "Data/Items/Effects/HealPickupEffect.h"
#include "Actors/Components/EntityVitalsComponent.h"
#include "MyTournamentGameMode.h"

// Sets default values
APickupBase::APickupBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Root for the actor
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	check(_mesh != nullptr);
	_mesh->SetupAttachment(RootComponent);

	// Collider for pickup
	_collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));
	check(_collider != nullptr);
	_collider->SetSphereRadius(50.0f);
	_collider->SetupAttachment(_mesh);

	_glowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlowMesh"));
	check(_glowMesh != nullptr);
	_glowMesh->SetupAttachment(RootComponent);

	_audioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	check(_audioComponent != nullptr);
	_audioComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();

	Initialize();

	// Register this pickup
	if (AMyTournamentGameMode* gm = GetWorld()->GetAuthGameMode<AMyTournamentGameMode>())
	{
		bool bHasHealEffect = false;

		for (const UPickupEffect* effect : this->_itemAsset->_effects)
		{
			if (effect && effect->IsA(UHealPickupEffect::StaticClass()))
			{
				bHasHealEffect = true;
				break;
			}
		}

		if (this->_itemAsset->_type == EItemType::Consumable && bHasHealEffect)
		{
			gm->RegisterHealthPickup(this);
		}
	}
}

void APickupBase::Initialize()
{
	if (_itemAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("Pickup %s doesn't have an _itemAsset assigned!"), *GetActorNameOrLabel());
		return;
	}

	if (!_itemAsset.IsValid())
		_itemAsset.LoadSynchronous();

	// _itemAsset is now safe to consider loaded, but not all of its properties
	if (!_itemAsset->_meshPrimary.IsValid())
		_itemAsset->_meshPrimary.LoadSynchronous(); // Load mesh 

	_mesh->SetStaticMesh(_itemAsset->_meshPrimary.Get());
	_mesh->SetVisibility(true);

	// Mesh's collider will not determine collision, but _collider will
	_mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_mesh->SetGenerateOverlapEvents(false);

	_collider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	_collider->SetGenerateOverlapEvents(true);
	_collider->OnComponentBeginOverlap.AddUniqueDynamic(this, &APickupBase::OnColliderBeginOverlap);

	// Set glowMesh default material
	if(_glowMesh)
		_glowMesh->SetMaterial(0, _glowMeshMaterialAvailable);

	_startMeshLocation = _mesh->GetComponentLocation();

	_audioComponent->SetSound(_pickupSound);

	_isCurrentlyActive = true;
}

// Called every frame
void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_doesRotate)
		_mesh->AddWorldRotation(FQuat(FVector(0, 0, 1), _rotSpeed * DeltaTime));

	if (_doesHover)
	{
		_hoverTime += DeltaTime;

		// Sinusoidal offset on Z
		const float zOffset = _hoverRange * FMath::Sin(2.f * PI * _hoverSpeed * _hoverTime);

		// Set absolute position from the stored start position (prevents drift)
		_mesh->SetWorldLocation(_startMeshLocation + FVector(0.f, 0.f, zOffset));
	}
}

void APickupBase::OnColliderBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if it was the player
	UEntityVitalsComponent* vitals = OtherActor->FindComponentByClass<UEntityVitalsComponent>();
	if (vitals)
	{
		// Checks for Pickup conditions (max health, max armor, etc.)
		if (!CanBeTakenBy(OtherActor))
		{
			return;
		}

		TakePickup(OtherActor);

		// Deactivate the pickup 
		_collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		_collider->SetGenerateOverlapEvents(false);
		_mesh->SetVisibility(false);
		_isCurrentlyActive = false;

		// Call for respawn
		if (_doesRespawn)
		{
			GetWorldTimerManager().SetTimer(_respawnTimeHandle, this, &APickupBase::RespawnPickup, _respawnTime, false);

			// Set glowMesh material
			if (_glowMesh)
				_glowMesh->SetMaterial(0, _glowMeshMaterialRespawning);
		}
		else if(_glowMesh)
			_glowMesh->SetVisibility(false);
	}
}

void APickupBase::TakePickup(AActor* taker)
{
	_audioComponent->Play();

	// Apply effects to actor
	for (int i = 0; i < _itemAsset->_effects.Num(); i++)
	{
		_itemAsset->_effects[i]->ApplyTo(taker);
	}
}

void APickupBase::RespawnPickup()
{
	_collider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	_collider->SetGenerateOverlapEvents(true);
	_mesh->SetVisibility(true);
	_isCurrentlyActive = true;

	// Set glowMesh material
	if (_glowMesh)
		_glowMesh->SetMaterial(0, _glowMeshMaterialAvailable);
}

bool APickupBase::CanBeTakenBy(AActor* actor)
{
	if (_itemAsset->_pickupConditionsMask == 0)
	{
		return true;
	}

	// Check for vitals component, if it's there, check the conditions
	if (UEntityVitalsComponent* vitals = actor->FindComponentByClass<UEntityVitalsComponent>())
	{
		if (_itemAsset->HasPickupCondition(EPickupCondition::DoNotTake_OnHealthIsMax))
			return !IDamageable::Execute_IsAtMaxHealth(vitals);

		if (_itemAsset->HasPickupCondition(EPickupCondition::DoNotTake_OnArmorIsMax))
			return !IDamageable::Execute_IsAtMaxArmor(vitals);

		if (_itemAsset->HasPickupCondition(EPickupCondition::DoNotTake_OnBothHealthAndArmorIsMax))
			return (!IDamageable::Execute_IsAtMaxHealth(vitals) || !IDamageable::Execute_IsAtMaxArmor(vitals));
	}
	
	return false;
}

bool APickupBase::IsCurrentlyActive()
{
	return _isCurrentlyActive;
}

void APickupBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// Handle parent class property changes
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName propertyChanged = PropertyChangedEvent.Property ? PropertyChangedEvent.GetPropertyName() : NAME_None;

	if (propertyChanged == GET_MEMBER_NAME_CHECKED(APickupBase, _itemAsset))
	{
		if (_itemAsset)
		{
			if (!_itemAsset.IsValid())
				_itemAsset.LoadSynchronous();

			// _itemAsset is now safe to consider loaded, but not all of its properties
			if (!_itemAsset->_meshPrimary.IsValid())
				_itemAsset->_meshPrimary.LoadSynchronous(); // Load mesh 

			_mesh->SetStaticMesh(_itemAsset->_meshPrimary.Get());
			_mesh->SetVisibility(true);
		}
	}
}
