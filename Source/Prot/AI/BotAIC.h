// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Engine/TargetPoint.h"
#include "BotAIC.generated.h"

/**
 * 
 */
UCLASS()
class PROT_API ABotAIC : public AAIController
{
	GENERATED_BODY()

	ABotAIC();

	//Behavior tree component reference
	UBehaviorTreeComponent* BehaviorComp;

	//Blackboard component reference
	UBlackboardComponent* BlackboardComp;

	/*Posses is executed when the character we want to control is spawned.
	Inside this function, we initialize the blackboard and start the behavior tree*/
	virtual void Possess(APawn* Pawn) override;

public:
	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const 
	{
		return BlackboardComp;
	}

	virtual ATargetPoint* GetTargetPointByIndex(int index) const;
	virtual int GetTargetPointsNumber();
};
