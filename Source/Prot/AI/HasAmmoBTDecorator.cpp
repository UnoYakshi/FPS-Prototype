// Fill out your copyright notice in the Description page of Project Settings.

#include "HasAmmoBTDecorator.h"
#include "BotAIC.h"
#include "Bot.h"
#include "Weapons/Weapon.h"

bool UHasAmmoBTDecorator::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool HasAmmo = false;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (BlackboardComponent)
	{
		ABot* Bot = Cast<ABot>(BlackboardComponent->GetValueAsObject("SelfActor"));
		if (Bot)
		{
			AWeapon* Weapon = Bot->GetCurrentWeapon();
			if (Weapon)
			{
				HasAmmo = Weapon->HasAmmo();
			}
		}
	}
	return HasAmmo;
}
