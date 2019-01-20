// Fill out your copyright notice in the Description page of Project Settings.

#include "BotAIC.h"
#include "Bot.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"

ABotAIC::ABotAIC()
{
	//Initialize BehaviorTreeComponent, BlackboardComponent
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void ABotAIC::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);

	//Get the possessed Pawn and check if it's Bot
	ABot* Bot = Cast<ABot>(Pawn);
	if (Bot)
	{
		//If the blackboard is valid initialize the blackboard for the corresponding behavior tree
		if (Bot->BehaviorTree->BlackboardAsset)
		{
			BlackboardComponent->InitializeBlackboard(*(Bot->BehaviorTree->BlackboardAsset));
			BlackboardComponent->SetValueAsInt("PointIndex", 0);

			//Start the behavior tree which corresponds to the specific character
			BehaviorTreeComponent->StartTree(*Bot->BehaviorTree);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Blackboard was not valid"));
		}
	}
}

ATargetPoint* ABotAIC::GetTargetPointByIndex(int index) const
{
	ABot* Bot = Cast<ABot>(GetPawn());
	if (Bot && index < Bot->PatrolPoints.Num() && index >= 0)
	{
		return Bot->PatrolPoints[index];
	}
	return nullptr;
}

int ABotAIC::GetTargetPointsNumber()
{
	ABot* Bot = Cast<ABot>(GetPawn());
	if (Bot)
	{
		return Bot->PatrolPoints.Num();
	}
	return 0;
}
