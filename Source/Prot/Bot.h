// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PatrolPoint.h"
#include "NavigationPath.h"
#include "GameFramework/PawnMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Bot.generated.h"

UCLASS()
class PROT_API ABot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Points to patrol
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TArray<APatrolPoint*> PatrolPoints;

	// Bot's mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	USkeletalMeshComponent* BotMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBehaviorTree* BehaviorTree;

	class UPawnMovementComponent* BotMovementComponent;

	virtual UPawnMovementComponent* GetMovementComponent() const override;
};
