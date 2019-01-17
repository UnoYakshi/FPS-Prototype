// Fill out your copyright notice in the Description page of Project Settings.

#include "Bot.h"
#include "NavigationSystem.h"
#include "PatrolPoint.h"
#include "DrawDebugHelpers.h"
#include "NavigationPath.h"
#include "GameFramework/PawnMovementComponent.h"

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
	
	BotMovementComponent = CreateDefaultSubobject<UPawnMovementComponent>(TEXT("BotMovementComponent"));
	BotMovementComponent->UpdatedComponent = RootComponent;
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

UPawnMovementComponent* ABot::GetMovementComponent() const
{
	return BotMovementComponent;
}