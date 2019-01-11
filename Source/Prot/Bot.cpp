// Fill out your copyright notice in the Description page of Project Settings.

#include "Bot.h"
#include "NavigationSystem.h"
#include "PatrolPoint.h"
#include "DrawDebugHelpers.h"
#include "NavigationPath.h"

// Sets default values
ABot::ABot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BotMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BotMesh"));
	if (BotMesh)
	{
		BotMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, NAME_None);
	}
	CurrentPointIndex = 0;
}

// Called when the game starts or when spawned
void ABot::BeginPlay()
{
	Super::BeginPlay();
	MoveToNextPoint();
}

// Called every frame
void ABot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABot::MoveToNextPoint()
{
	++CurrentPointIndex;
	if (CurrentPointIndex == PatrolPoints.Num())
	{
		CurrentPointIndex = 0;
	}

	APatrolPoint *NextPatrolPoint = PatrolPoints[CurrentPointIndex];
	UWorld *World = GetWorld();
	UNavigationPath *Path = UNavigationSystemV1::FindPathToActorSynchronously(World, GetActorLocation(), NextPatrolPoint);
	TArray<FVector> Points = Path->PathPoints;
	
	//TODO movement
	for (int i = 0; i < Points.Num(); ++i)
	{
		DrawDebugBox(World, Points[i], FVector(1.0f, 1.0f, 1.0f), FColor::White);
	}
}