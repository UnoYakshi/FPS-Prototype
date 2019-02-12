// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "HasAmmoBTDecorator.generated.h"

/**
 * Decorator that tells if Bot's weapon is loaded
 */
UCLASS()
class PROT_API UHasAmmoBTDecorator : public UBTDecorator
{
	GENERATED_BODY()
public:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
