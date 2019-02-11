// Fill out your copyright notice in the Description page of Project Settings.
#include "HasWeaponBTDecorator.h"
#include "BotAIC.h"
#include "Bot.h"
#include "Weapons/Weapon.h"

bool UHasWeaponBTDecorator::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool HasWeapon = false;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (BlackboardComponent)
	{
		ABot* Bot = Cast<ABot>(BlackboardComponent->GetValueAsObject("SelfActor"));
		if (Bot)
		{
			AWeapon* Weapon = Bot->GetCurrentWeapon();
			HasWeapon = Weapon != nullptr;
		}
	}
	return HasWeapon;
}
