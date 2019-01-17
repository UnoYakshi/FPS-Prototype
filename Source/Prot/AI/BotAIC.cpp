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
	//Initialize BehaviorTreeComponent, BlackboardComponent and the corresponding key

	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));

	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

	LocationToGoKey = "LocationToGo";

}

void ABotAIC::Possess(APawn* Pawn)
{
	Super::Possess(Pawn);

	//Get the possessed Character and check if it's my own AI Character
	ABot* Bot = Cast<ABot>(Pawn);

	if (Bot)
	{
		//If the blackboard is valid initialize the blackboard for the corresponding behavior tree
		if (Bot->BehaviorTree->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*(Bot->BehaviorTree->BlackboardAsset));
		}

		/*Populate the array of available bot target points
		The following function needs a TArray of AActors, that's why I declared the
		BotTargetPoints as such. When I will need to get an exact point and compare it,
		I will cast it to the corresponding class (ABotTargetPoint)*/
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), BotTargetPoints);

		//Start the behavior tree which corresponds to the specific character
		BehaviorComp->StartTree(*Bot->BehaviorTree);
	}
}
