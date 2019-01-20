// Fill out your copyright notice in the Description page of Project Settings.

#include "Bot.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "NavigationPath.h"
#include "GameFramework/CharacterMovementComponent.h"

ABot::ABot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ABot::BeginPlay()
{
	Super::BeginPlay();
}

void ABot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
