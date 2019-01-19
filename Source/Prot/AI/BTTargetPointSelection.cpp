// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTargetPointSelection.h"
#include "BotAIC.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/TargetPoint.h"

EBTNodeResult::Type UBTTargetPointSelection::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	//AI Contoller that owns the Behavior Tree from which this Task Node is executed
	ABotAIC* AIController = Cast<ABotAIC>(OwnerComp.GetAIOwner());
	if (AIController)
	{
		UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComp();

		//Update the Target Point in the Blackboard so the bot can move to the next Target Point
		int PointIndex = BlackboardComp->GetValueAsInt("PointIndex");
		BlackboardComp->SetValueAsObject("LocationToGo", AIController->GetTargetPointByIndex(PointIndex));
		PointIndex++;
		if (PointIndex > AIController->GetTargetPointsNumber())
		{
			PointIndex = 0;
		}
		BlackboardComp->SetValueAsInt("PointIndex", PointIndex);

		//At this point, the task has been successfully completed
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
