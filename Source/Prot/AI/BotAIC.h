// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Engine/TargetPoint.h"
#include "BotAIC.generated.h"

//Bot Controller class that is used to get access for Bot's data from Behavior Tree 
UCLASS()
class PROT_API ABotAIC : public AAIController
{
	GENERATED_BODY()

	ABotAIC();

	//For starting Behavior Tree
	UBehaviorTreeComponent* BehaviorTreeComponent;

	//For initializing Blackboard and its Values
	UBlackboardComponent* BlackboardComponent;

	/*Posses is executed when the character we want to control is spawned.
	Inside this function, we initialize the blackboard and start the behavior tree*/
	virtual void Possess(APawn* Pawn) override;

public:
	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const 
	{
		return BlackboardComponent;
	}

	//Gets Target Point from Bot's array by index
	virtual ATargetPoint* GetTargetPointByIndex(int index) const;

	//Gets number of Target Points in Bot's array
	virtual int GetTargetPointsNumber();
};
