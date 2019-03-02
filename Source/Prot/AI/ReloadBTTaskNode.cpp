// Fill out your copyright notice in the Description page of Project Settings.

#include "ReloadBTTaskNode.h"
#include "BotAIC.h"
#include "Bot.h"

EBTNodeResult::Type UReloadBTTaskNode::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (BlackboardComponent)
	{
		ABot* Bot = Cast<ABot>(BlackboardComponent->GetValueAsObject("SelfActor"));
		if (Bot && Bot->GetCurrentWeapon())
		{
			Bot->CurrentWeaponComp->Reload();
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}
