// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Bot.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Engine/TargetPoint.h"
#include "BotAIC.generated.h"

// Bot Controller class that is used to get access for Bot's data from Behavior Tree.
UCLASS()
class PROT_API ABotAIC : public AAIController
{
	GENERATED_BODY()

	ABotAIC();

	// For starting Behavior Tree
	UBehaviorTreeComponent* BehaviorTreeComponent;

	// For initializing Blackboard and its Values
	UBlackboardComponent* BlackboardComponent;

	// For setting seeing timer
	float SensingInterval;

	/**
	 * Posses is executed when the character we want to control is spawned.
	 * Inside this function, we initialize the blackboard and start the behavior tree 
	 */
	virtual void Possess(APawn* Pawn) override;

	// Bot that is currently possessed by controller
	ABot* Bot;

	// Handle to manage bot seeing timer
	FTimerHandle SeenTimerHandle;

	// Handle to manage bot seeing timer
	FTimerHandle HeardTimerHandle;

	// Handle to manage bot shooting timer
	FTimerHandle FireTimerHandle;

	// Called when bot stops seeing the player
	virtual void OnBotUnSee();

	// Called when bot sees the player
	UFUNCTION()
	virtual void OnBotSee(APawn* SeenPawn);

	/* Hearing function - will be executed when we hear a Pawn */
	UFUNCTION()
	virtual void OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume);

	// Called so Bot stops hearing the player
	virtual void OnUnHearNoise();

	// Stops Bot's BehaviorTree
	UFUNCTION()
	virtual void StopBotBehaviorTree(AActor* DeadBot);

	// Stops Bot shooting
	virtual void StopBotShooting();

public:
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const
	{
		return BlackboardComponent;
	}

	// Gets Target Point from Bot's array by index
	virtual ATargetPoint* GetTargetPointByIndex(int index) const;

	// Gets number of Target Points in Bot's array
	virtual int GetTargetPointsNumber();

	// Is patrolling order of Bot random?
	virtual bool PatrolRandomly();

	// Set timer that stops Bot shooting
	virtual void SetStopShootingTimer();
};
