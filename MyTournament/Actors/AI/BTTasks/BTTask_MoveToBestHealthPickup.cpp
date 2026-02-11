// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MoveToBestHealthPickup.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../AIControllerMyTournamentBot.h"
#include "../AIMyTournamentBot.h"
#include "../../Items/PickupBase.h"
#include "../../../MyTournamentGameMode.h"

UBTTask_MoveToBestHealthPickup::UBTTask_MoveToBestHealthPickup()
{
	NodeName = TEXT("Move To Best Health Pickup");
	bCreateNodeInstance = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveToBestHealthPickup::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	_ownerComp = &OwnerComp;
	_aiController = OwnerComp.GetAIOwner();
	_pawn = _aiController ? _aiController->GetPawn() : nullptr;

	_blackboard = OwnerComp.GetBlackboardComponent();

	if (!_aiController || !_pawn || !_blackboard)
		return EBTNodeResult::Failed;

	UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(_pawn->GetWorld());

	if (!navSys)
		return EBTNodeResult::Failed;

	// Get closest health pickup 
	AMyTournamentGameMode* gm = Cast<AMyTournamentGameMode>(GetWorld()->GetAuthGameMode());
	FPathFollowingRequestResult moveResult = FindNewPickupAndMoveTo(gm->GetBestHealthPickup(_pawn->GetActorLocation()));

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
	_aiController->ReceiveMoveCompleted.AddUniqueDynamic(this, &UBTTask_MoveToBestHealthPickup::OnMoveCompleted);
	isCurrentlyInAMove = true;

	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToBestHealthPickup::OnMoveCompleted(FAIRequestID requestID, EPathFollowingResult::Type result)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("OnMoveCompleted!"));

	if (requestID != _moveRequestID || !_ownerComp)
		return;

	_aiController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_MoveToBestHealthPickup::OnMoveCompleted);
	isCurrentlyInAMove = false;

	if (result == EPathFollowingResult::Success)
		this->FinishLatentTask(*this->_ownerComp, EBTNodeResult::Succeeded);
	else
		this->FinishLatentTask(*this->_ownerComp, EBTNodeResult::Failed);
}

FPathFollowingRequestResult UBTTask_MoveToBestHealthPickup::FindNewPickupAndMoveTo(APickupBase* pickup)
{
	// Get closest health pickup 
	_currentPickup = pickup;

	if (!_currentPickup)
		this->FinishLatentTask(*this->_ownerComp, EBTNodeResult::Failed);

	_blackboard->SetValueAsObject(FName("CurrentPickupTarget"), _currentPickup);
	_blackboard->SetValueAsVector(FName("TargetVector"), _currentPickup->GetActorLocation());

	// AI MoveTo
	FAIMoveRequest moveReq;
	moveReq.SetGoalLocation(_currentPickup->GetActorLocation());
	moveReq.SetAcceptanceRadius(_acceptanceRadius);
	moveReq.SetUsePathfinding(true);
	moveReq.SetAllowPartialPath(true);

	FPathFollowingRequestResult moveResult = _aiController->MoveTo(moveReq);
	_moveRequestID = moveResult.MoveId;

	return moveResult;
}

void UBTTask_MoveToBestHealthPickup::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Reaching Tick"));

	// If the current pickup is taken by somebody else, try to find another one
	if (_currentPickup && !_currentPickup->IsCurrentlyActive())
	{
		_aiController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_MoveToBestHealthPickup::OnMoveCompleted);
		isCurrentlyInAMove = false;

		// Get closest health pickup 
		AMyTournamentGameMode* gm = Cast<AMyTournamentGameMode>(GetWorld()->GetAuthGameMode());
		FPathFollowingRequestResult moveResult = FindNewPickupAndMoveTo(gm->GetBestHealthPickup(_pawn->GetActorLocation()));

		// Early exits
		if (moveResult.Code == EPathFollowingRequestResult::Failed)
		{
			this->FinishLatentTask(*this->_ownerComp, EBTNodeResult::Failed);
		}

		if (moveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			this->FinishLatentTask(*this->_ownerComp, EBTNodeResult::Succeeded);
		}

		// Latent task
		_aiController->ReceiveMoveCompleted.AddUniqueDynamic(this, &UBTTask_MoveToBestHealthPickup::OnMoveCompleted);
		isCurrentlyInAMove = true;
	}
}
