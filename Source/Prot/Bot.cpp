// Fill out your copyright notice in the Description page of Project Settings.

#include "Bot.h"
#include "NavigationSystem.h"
#include "PatrolPoint.h"
#include "DrawDebugHelpers.h"
#include "NavigationPath.h"
#include "Kismet/KismetMathLibrary.h"

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
	CurrentPatrolPointIndex = 0;
	CurrentPathPointIndex = 0;
	PathToNextPatrolPoint = NULL;
}

// Called when the game starts or when spawned
void ABot::BeginPlay()
{
	Super::BeginPlay();
	MoveToNextPatrolPoint();
}

// Called every frame
void ABot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PathToNextPatrolPoint && PathToNextPatrolPoint->PathPoints.Num() > 0) 
	{
		FVector DirectionUnitVector = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(),
			PathToNextPatrolPoint->PathPoints[CurrentPathPointIndex]);
		FVector Delta = DirectionUnitVector * DeltaTime*Speed;
		if (Delta.Size() > (GetActorLocation() - PathToNextPatrolPoint->PathPoints[CurrentPathPointIndex]).Size()) {
			Delta = DirectionUnitVector*(GetActorLocation() - PathToNextPatrolPoint->PathPoints[CurrentPathPointIndex]).Size();
		}
		SetActorLocation(GetActorLocation() + Delta);
		SetNextPathPoint();
		if (CurrentPathPointIndex == PathToNextPatrolPoint->PathPoints.Num()) {
			CurrentPathPointIndex = 0;
			MoveToNextPatrolPoint();
		}
	}
}

// Called to bind functionality to input
void ABot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABot::SetNextPathPoint()
{
	if ((GetActorLocation() - PathToNextPatrolPoint->PathPoints[CurrentPathPointIndex]).Size() < 0.001)
	{
		CurrentPathPointIndex++;
	}
}

void ABot::MoveToNextPatrolPoint()
{
	if (PatrolPoints.Num() > 0)
	{
		APatrolPoint *NextPatrolPoint = PatrolPoints[CurrentPatrolPointIndex++];
		if (CurrentPatrolPointIndex == PatrolPoints.Num())
		{
			CurrentPatrolPointIndex = 0;
		}

		UWorld *World = GetWorld();
		PathToNextPatrolPoint = UNavigationSystemV1::FindPathToLocationSynchronously(World, GetActorLocation(), NextPatrolPoint->GetActorLocation(), NULL);
		if (PathToNextPatrolPoint)
		{	
			TArray<FVector> Points = PathToNextPatrolPoint->PathPoints;
			for (int i = 0; i < Points.Num(); ++i)
			{
				DrawDebugSphere(World, Points[i], 10.f, 6, FColor::White, true, 5.0f, 0, 2.f);
				//SetActorLocation(Points[i]);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Creation of path to Patrol Point failed"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("The array of Patrol Points is empty"));
	}
}
