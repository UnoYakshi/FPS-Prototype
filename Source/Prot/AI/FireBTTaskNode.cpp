// Fill out your copyright notice in the Description page of Project Settings.

#include "FireBTTaskNode.h"
#include "BotAIC.h"

EBTNodeResult::Type UFireBTTaskNode::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (BlackboardComponent)
	{
		ABot* ShootingBot = Cast<ABot>(BlackboardComponent->GetValueAsObject("SelfActor"));
		if (ShootingBot)
		{
			Bot = ShootingBot;
			AWeapon* Weapon = Bot->GetCurrentWeapon();
			if (Weapon && Weapon->CanFire())
			{
				Bot->CurrentWeaponComp->TryStartFire();
				UWorld* World = GetWorld();
				if (World)
				{
					World->GetTimerManager().SetTimer(FireTimerHandle, this, &UFireBTTaskNode::StopShooting,
						1.f, false);
				}
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
}

void UFireBTTaskNode::StopShooting()
{
	if (Bot && Bot->GetCurrentWeapon())
	{
		Bot->CurrentWeaponComp->TryStopFire();
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(FireTimerHandle);
		}
	}
}
