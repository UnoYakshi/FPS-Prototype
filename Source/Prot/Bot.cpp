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
	BotMesh->SetupAttachment(RootComponent);

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
	// @Oninbo, fix CurrentPointIndex handling: there will be IndexOutOfRange stuff if you won't clamp the value...
	++CurrentPointIndex;
	APatrolPoint *NextPatrolPoint = PatrolPoints[CurrentPointIndex];
	UNavigationPath *Path = UNavigationSystemV1::FindPathToActorSynchronously(GetWorld(), GetActorLocation(), NextPatrolPoint);

	//TODO movement
}