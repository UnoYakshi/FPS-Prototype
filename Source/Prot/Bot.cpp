// Fill out your copyright notice in the Description page of Project Settings.

#include "Bot.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "NavigationPath.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ABot::ABot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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