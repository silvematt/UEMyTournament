// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_MoveToRandomSpotOnNavmesh.generated.h"

/**
 * 
 */
UCLASS()
class MYTOURNAMENT_API UBTTask_MoveToRandomSpotOnNavmesh : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_MoveToRandomSpotOnNavmesh();

// Properties
public:
	// Cache
	AAIController* _aiController;
	APawn* _pawn;
	UBehaviorTreeComponent* _ownerComp;
	FAIRequestID _moveRequestID;
	UBlackboardComponent* _blackboard;

	UPROPERTY(EditAnywhere, Category = "Search")
	float _searchRadius = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Search")
	float _acceptanceRadius = 100.0f;
	
// Methods
public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UFUNCTION()
	void OnMoveCompleted(FAIRequestID requestID, EPathFollowingResult::Type result);
};
