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
	APatrolPoint *NextPatrolPoint = PatrolPoints[CurrentPointIndex++];
	if (CurrentPointIndex == PatrolPoints.Num())
	{
		CurrentPointIndex = 0;
	}

	UWorld *World = GetWorld();
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(World, GetActorLocation(), NextPatrolPoint->GetActorLocation(), NULL);
	if (Path)
	{
		TArray<FVector> Points = Path->PathPoints;
		for (int i = 0; i < Points.Num(); ++i)
		{
			DrawDebugSphere(World, Points[i], 10.f, 6, FColor::White, true, 21.f, 0, 2.f);
		}

		//TODO movement
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SS"));
	}
}
