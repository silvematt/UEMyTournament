// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupBase.h"
#include "../../Data/Items/ItemAsset.h"

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

	_glowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlowMesh"));
	check(_glowMesh != nullptr);
	_glowMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();

	Initialize();
	
	_startMeshLocation = _mesh->GetComponentLocation();
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

	// Mesh's collider will determine overlaps
	_mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	_mesh->SetGenerateOverlapEvents(true);
	_mesh->OnComponentBeginOverlap.AddDynamic(this, &APickupBase::OnMeshBeginOverlap);

	// Set glowMesh default material
	_glowMesh->SetMaterial(0, _glowMeshMaterialAvailable);
}

void APickupBase::OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if it was the player
	AMyTournamentCharacter* myChar = Cast<AMyTournamentCharacter>(OtherActor);
	if (myChar)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Player took pickup %s which is %s"), *GetName(), *_itemAsset->_ID.ToString()));

		TakePickup(OtherActor);

		// Deactivate the pickup 
		_mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		_mesh->SetGenerateOverlapEvents(false);
		_mesh->SetVisibility(false);

		// Call for respawn
		if (_doesRespawn)
		{
			GetWorldTimerManager().SetTimer(_respawnTimeHandle, this, &APickupBase::RespawnPickup, _respawnTime, false);

			// Set glowMesh material
			_glowMesh->SetMaterial(0, _glowMeshMaterialRespawning);
		}
		else
			_glowMesh->SetVisibility(false);
	}
}

void APickupBase::TakePickup(AActor* taker)
{
	// Apply effects to actor
	for (int i = 0; i < _itemAsset->_effects.Num(); i++)
	{
		_itemAsset->_effects[i]->ApplyTo(taker);
	}
}

void APickupBase::RespawnPickup()
{
	_mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	_mesh->SetGenerateOverlapEvents(true);
	_mesh->SetVisibility(true);

	// Set glowMesh material
	_glowMesh->SetMaterial(0, _glowMeshMaterialAvailable);
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
