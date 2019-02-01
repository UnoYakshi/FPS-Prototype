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
#include "Perception/PawnSensingComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"

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
	Bot = Cast<ABot>(Pawn);
	if (Bot)
	{
		//If the blackboard is valid initialize the blackboard for the corresponding behavior tree
		if (Bot->BehaviorTree->BlackboardAsset)
		{
			BlackboardComponent->InitializeBlackboard(*(Bot->BehaviorTree->BlackboardAsset));
			BlackboardComponent->SetValueAsInt("PointIndex", 0);
			BlackboardComponent->SetValueAsBool("False", false);

			//Start the behavior tree which corresponds to the specific character
			BehaviorTreeComponent->StartTree(*Bot->BehaviorTree);

			UPawnSensingComponent* SensingComponent =
				Cast<UPawnSensingComponent>(Bot->GetComponentByClass(UPawnSensingComponent::StaticClass()));
			if (SensingComponent)
			{
				SensingInterval = SensingComponent->SensingInterval;
				SensingComponent->OnSeePawn.AddDynamic(this, &ABotAIC::OnBotSee);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Blackboard was not valid"));
		}

	}
}

ATargetPoint* ABotAIC::GetTargetPointByIndex(int index) const
{
	if (Bot && index < Bot->PatrolPoints.Num() && index >= 0)
	{
		return Bot->PatrolPoints[index];
	}
	return nullptr;
}

int ABotAIC::GetTargetPointsNumber()
{
	if (Bot)
	{
		return Bot->PatrolPoints.Num();
	}
	return 0;
}

void ABotAIC::OnBotUnSee()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Can't see you!"));
	BlackboardComponent->SetValueAsBool("isPlayerSeen", false);
}

void ABotAIC::OnBotSee(APawn* SeenPawn)
{
	UWorld* World = GetWorld();
	if (World) 
	{
		World->GetTimerManager().SetTimer(SeenTimerHandle, this, &ABotAIC::OnBotUnSee,
			SensingInterval + 0.1f, false);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("I can see you!"));
		BlackboardComponent->SetValueAsBool("isPlayerSeen", true);
		BlackboardComponent->SetValueAsObject("TargetPlayer", SeenPawn);
	}
}
