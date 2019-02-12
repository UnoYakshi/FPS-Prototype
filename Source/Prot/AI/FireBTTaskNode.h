// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Weapons/Weapon.h"
#include "Bot.h"
#include "FireBTTaskNode.generated.h"

/**
 * Performs shooting from Bot's weapon.
 */
UCLASS()
class PROT_API UFireBTTaskNode : public UBTTaskNode
{
	GENERATED_BODY()
	// Handle to manage bot shooting timer
	FTimerHandle FireTimerHandle;
	ABot* Bot;
	virtual void StopShooting();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
