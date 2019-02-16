// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "HasWeaponBTDecorator.generated.h"

/**
 * Decorator that tells if Bot has a weapon
 */
UCLASS()
class PROT_API UHasWeaponBTDecorator : public UBTDecorator
{
	GENERATED_BODY()
public:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
