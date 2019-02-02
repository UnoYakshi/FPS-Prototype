// Fill out your copyright notice in the Description page of Project Settings.

#include "BTRunPointSelect.h"
#include "GameFramework/Actor.h"
#include "BotAIC.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"

EBTNodeResult::Type UBTRunPointSelect::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	ABotAIC* AIController = Cast<ABotAIC>(OwnerComp.GetAIOwner());
	if (AIController)
	{
		UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent();
		AActor* Player = Cast<AActor>(BlackboardComponent->GetValueAsObject("TargetPlayer"));
		AActor* Bot = Cast<AActor>(BlackboardComponent->GetValueAsObject("SelfActor"));
		if (Player && Bot)
		{
			// Random angle from -135 to 135 degrees
			float Angle = FMath::RandRange(-135.f, 135.f);
			// Direction from Player to Bot
			FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(Player->GetActorLocation(), Bot->GetActorLocation());
			// Rotate direction so Bot will run away from Player not towards
			Direction = Direction.RotateAngleAxis(Angle, FVector(0, 0, 1));
			// Create a point where Bot will run from Player
			BlackboardComponent->SetValueAsVector("RunLocation", Bot->GetActorLocation() + Direction * RunDistance);
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}