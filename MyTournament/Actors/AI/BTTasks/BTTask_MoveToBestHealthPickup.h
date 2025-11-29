// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_MoveToBestHealthPickup.generated.h"

class APickupBase;

/**
 * 
 */
UCLASS()
class MYTOURNAMENT_API UBTTask_MoveToBestHealthPickup : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_MoveToBestHealthPickup();

	// Cache
	AAIController* _aiController;
	APawn* _pawn;
	UBehaviorTreeComponent* _ownerComp;
	FAIRequestID _moveRequestID;
	UBlackboardComponent* _blackboard;
	APickupBase* _currentPickup;

	bool isCurrentlyInAMove = false;

	UPROPERTY(EditAnywhere, Category = "Search")
	float _acceptanceRadius = 100.0f;


public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UFUNCTION()
	void OnMoveCompleted(FAIRequestID requestID, EPathFollowingResult::Type result);

	FPathFollowingRequestResult FindNewPickupAndMoveTo(APickupBase* pickup);
};
