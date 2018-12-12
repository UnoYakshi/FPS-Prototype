// Fill out your copyright notice in the Description page of Project Settings.

#include "Bot.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "PatrolPoint.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"

// Sets default values
ABot::ABot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	UStaticMeshComponent* SphereVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
	SphereVisual->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	if (SphereVisualAsset.Succeeded())
	{
		SphereVisual->SetStaticMesh(SphereVisualAsset.Object);
		SphereVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
		SphereVisual->SetWorldScale3D(FVector(0.8f));
	}

	CurrentPointIndex = 0;
}

// Called when the game starts or when spawned
void ABot::BeginPlay()
{
	Super::BeginPlay();
	
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
	CurrentPointIndex++;
	APatrolPoint *NextPatrolPoint = PatrolPoints[CurrentPointIndex];
	UNavigationPath *Path = UNavigationSystemV1::FindPathToActorSynchronously(GetWorld(), GetActorLocation(), NextPatrolPoint);

	//FIXME compilation errors
	//TODO movement
}