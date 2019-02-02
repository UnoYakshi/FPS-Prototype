// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTRunPointSelect.generated.h"

/**
 * Used for finding the point to run to from Player
 */
UCLASS(config = Game)
class PROT_API UBTRunPointSelect : public UBTTaskNode
{
	GENERATED_BODY()
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	// The distance for Bot to run from Player
	UPROPERTY(config, Category = Node, EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", MakeStructureDefaultValue = "2000.0"))
	float RunDistance;
};
