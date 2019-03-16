// Fill out your copyright notice in the Description page of Project Settings.

#include "BotAIC.h"
#include "Prot.h"
#include "Engine/Engine.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/PawnSensingComponent.h"
#include "HealthActorComponent.h"


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

			// Set Bot that uses the blackboard
			BlackboardComponent->SetValueAsObject("SelfActor", Bot);

			// Start the behavior tree which corresponds to the specific character
			BehaviorTreeComponent->StartTree(*Bot->BehaviorTree);

			// Make BehaviorTree stop when Bot is dead
			UHealthActorComponent* BotHealhComponent =
				Cast<UHealthActorComponent>(Bot->GetComponentByClass(UHealthActorComponent::StaticClass()));
			if (BotHealhComponent)
			{
				BotHealhComponent->OnDeath.AddDynamic(this, &ABotAIC::StopBotBehaviorTree);
			}

			UPawnSensingComponent* SensingComponent =
				Cast<UPawnSensingComponent>(Bot->GetComponentByClass(UPawnSensingComponent::StaticClass()));
			if (SensingComponent)
			{
				SensingInterval = SensingComponent->SensingInterval;
				SensingComponent->OnSeePawn.AddDynamic(this, &ABotAIC::OnBotSee);
				// Registering the delegate which will fire when we hear something
				SensingComponent->OnHearNoise.AddDynamic(this, &ABotAIC::OnHearNoise);
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
		World->GetTimerManager().SetTimer(
            SeenTimerHandle,
			this,
			&ABotAIC::OnBotUnSee,
			SensingInterval + 0.1f,
			false);
		if (DEBUG)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("I can see you!"));
		}
		Bot->CameraLookAt(SeenPawn);
		BlackboardComponent->SetValueAsObject("TargetPlayer", SeenPawn);
	}
}

void ABotAIC::OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume)
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			HeardTimerHandle,
			this,
			&ABotAIC::OnUnHearNoise,
			SensingInterval + 0.1f,
			false);
		if (DEBUG)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("I can hear you!"));
		}
		BlackboardComponent->SetValueAsObject("HeardSomeone", PawnInstigator);
	}
}

void ABotAIC::OnUnHearNoise()
{
	if (DEBUG)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Can't hear you!"));
	}
	BlackboardComponent->SetValueAsObject("HeardSomeone", nullptr);
}

void ABotAIC::StopBotBehaviorTree(AActor* DeadBot)
{
	if (BehaviorTreeComponent)
	{
		BehaviorTreeComponent->StopTree();
	}
}

void ABotAIC::SetStopShootingTimer()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(FireTimerHandle, this, &ABotAIC::StopBotShooting,
			0.5f, false);
	}
}

void ABotAIC::StopBotShooting()
{
	ABot* Bot = Cast<ABot>(GetPawn());
	if (Bot && Bot->GetCurrentWeapon())
	{
		Bot->CurrentWeaponComp->TryStopFire();
		Bot->CurrentWeaponComp->StopAim();
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(FireTimerHandle);
		}
	}
}
