// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProtCharacter.h"
#include "NavigationPath.h"
#include "GameFramework/PawnMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Engine/TargetPoint.h"
#include "Bot.generated.h"

//This class is used to create Bots that can patrol Target Points
UCLASS()
class PROT_API ABot : public AProtCharacter
{
	GENERATED_BODY()

public:
	ABot();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// The class of Behavior Tree that describes Bot's logic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBehaviorTree* BehaviorTree;

	// Points that Bot will patrol in the same order as they are in the array
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<ATargetPoint*> PatrolPoints;

	// Is patrolling order random?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bPatrolRandomly;
};
