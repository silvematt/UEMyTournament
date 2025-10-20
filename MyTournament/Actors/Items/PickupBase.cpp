// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupBase.h"
#include "../../Data/Items/ItemAsset.h"

// Sets default values
APickupBase::APickupBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	check(_mesh != nullptr);
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();

	Initialize();
	
}

// Called every frame
void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	// _itemAsset is now safe to consider loaded, but not his properties
	if (!_itemAsset->_meshPrimary.IsValid())
		_itemAsset->_meshPrimary.LoadSynchronous(); // Load mesh 

	_mesh->SetStaticMesh(_itemAsset->_meshPrimary.Get());
	_mesh->SetVisibility(true);

	// Mesh's collider will determine overlaps
	_mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	_mesh->SetGenerateOverlapEvents(true);
	_mesh->OnComponentBeginOverlap.AddDynamic(this, &APickupBase::OnMeshBeginOverlap);

}

void APickupBase::OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if it was the player
	AMyTournamentCharacter* myChar = Cast<AMyTournamentCharacter>(OtherActor);
	if (myChar)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("Player took pickup %s which is %s"), *GetName(), *_itemAsset->_ID.ToString()));

	}
}
