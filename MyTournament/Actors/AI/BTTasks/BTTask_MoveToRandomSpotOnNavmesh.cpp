// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MoveToRandomSpotOnNavmesh.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "../AIControllerMyTournamentBot.h"
#include "../AIMyTournamentBot.h"

UBTTask_MoveToRandomSpotOnNavmesh::UBTTask_MoveToRandomSpotOnNavmesh()
{
	NodeName = TEXT("Move To Random Spot On Navmesh");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_MoveToRandomSpotOnNavmesh::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	_ownerComp = &OwnerComp;
	_aiController = OwnerComp.GetAIOwner();
	_pawn = _aiController ? _aiController->GetPawn() : nullptr;

	if (!_aiController || !_pawn)
		return EBTNodeResult::Failed;

	UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(_pawn->GetWorld());

	if (!navSys)
		return EBTNodeResult::Failed;

	// Get Random Reachable Point in radius
	FNavLocation randomLocation;
	if (!navSys->GetRandomReachablePointInRadius(_pawn->GetActorLocation(), _searchRadius, randomLocation))
		return EBTNodeResult::Failed;

	// AI MoveTo
	FAIMoveRequest moveReq;
	moveReq.SetGoalLocation(randomLocation.Location);
	moveReq.SetAcceptanceRadius(_acceptanceRadius);
	moveReq.SetUsePathfinding(true);
	moveReq.SetAllowPartialPath(true);

	FPathFollowingRequestResult moveResult = _aiController->MoveTo(moveReq);
	_moveRequestID = moveResult.MoveId;

	// Early exits
	if (moveResult.Code == EPathFollowingRequestResult::Failed)
	{
		return EBTNodeResult::Failed;
	}

	if (moveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	// Latent task
	_aiController->ReceiveMoveCompleted.AddDynamic(this, &UBTTask_MoveToRandomSpotOnNavmesh::OnMoveCompleted);
	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToRandomSpotOnNavmesh::OnMoveCompleted(FAIRequestID requestID, EPathFollowingResult::Type result)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("OnMoveCompleted!"));

	if (requestID != _moveRequestID || !_ownerComp)
		return;

	_aiController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_MoveToRandomSpotOnNavmesh::OnMoveCompleted);

	if (result == EPathFollowingResult::Success)
		this->FinishLatentTask(*this->_ownerComp, EBTNodeResult::Succeeded);
	else
		this->FinishLatentTask(*this->_ownerComp, EBTNodeResult::Failed);
}
