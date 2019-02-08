// Fill out your copyright notice in the Description page of Project Settings.

#include "BotAIC.h"
#include "Engine/Engine.h"
#include "Prot.h"
#include "Engine/TargetPoint.h"
#include "Perception/PawnSensingComponent.h"

ABotAIC::ABotAIC()
{
	// Initialize BehaviorTreeComponent, BlackboardComponent
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void ABotAIC::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);

	// Get the possessed Pawn and check if it's Bot
	Bot = Cast<ABot>(Pawn);
	if (Bot)
	{
		// If the blackboard is valid initialize the blackboard for the corresponding behavior tree
		if (Bot->BehaviorTree && Bot->BehaviorTree->BlackboardAsset)
		{
			BlackboardComponent->InitializeBlackboard(*(Bot->BehaviorTree->BlackboardAsset));
			BlackboardComponent->SetValueAsInt("PointIndex", 0);

			// Start the behavior tree which corresponds to the specific character
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
			if (DEBUG)
			{ 
				UE_LOG(LogTemp, Error, TEXT("Blackboard was not valid"));
			}
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

bool ABotAIC::PatrolRandomly()
{
	if (Bot)
	{
		return Bot->bPatrolRandomly;
	}
	return false;
}

void ABotAIC::OnBotUnSee()
{
	if (DEBUG)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Can't see you!"));
	}
	BlackboardComponent->SetValueAsObject("TargetPlayer", nullptr);
}

void ABotAIC::OnBotSee(APawn* SeenPawn)
{
	UWorld* World = GetWorld();
	if (World) 
	{
		World->GetTimerManager().SetTimer(SeenTimerHandle,
			this,
			&ABotAIC::OnBotUnSee,
			SensingInterval + 0.1f,
			false);
		if (DEBUG)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("I can see you!"));
		}
		BlackboardComponent->SetValueAsObject("TargetPlayer", SeenPawn);
	}
}
