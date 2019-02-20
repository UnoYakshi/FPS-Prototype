// Fill out your copyright notice in the Description page of Project Settings.

#include "FireBTTaskNode.h"
#include "BotAIC.h"

EBTNodeResult::Type UFireBTTaskNode::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (BlackboardComponent)
	{
		ABot* Bot = Cast<ABot>(BlackboardComponent->GetValueAsObject("SelfActor"));
		if (Bot)
		{
			AWeapon* Weapon = Bot->GetCurrentWeapon();
			if (Weapon && Weapon->CanFire())
			{
				Bot->TryStartFire();
				UWorld* World = GetWorld();
				ABotAIC* BotController = Cast<ABotAIC>(OwnerComp.GetAIOwner());
				if (World && BotController)
				{
					BotController->SetStopShootingTimer();
				}
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
}
