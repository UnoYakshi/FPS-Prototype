// Fill out your copyright notice in the Description page of Project Settings.

#include "BTRunPointSelect.h"
#include "GameFramework/Actor.h"
#include "BotAIC.h"
#include "BehaviorTree/BlackboardComponent.h"
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
			float angle = FMath::RandRange(-135.f, 135.f);
			FVector direction = UKismetMathLibrary::GetDirectionUnitVector(Player->GetActorLocation(), Bot->GetActorLocation());
			direction = direction.RotateAngleAxis(angle, FVector(0, 0, 1));
			BlackboardComponent->SetValueAsVector("RunLocation", Bot->GetActorLocation() + direction * RunDistance);
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}