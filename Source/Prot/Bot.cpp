// Fill out your copyright notice in the Description page of Project Settings.

#include "Bot.h"
#include "Camera/CameraComponent.h"
#include "Weapons/WeaponComponent.h"

ABot::ABot()
{
	FPPCamera->AttachTo(RootComponent);
}

void ABot::BeginPlay()
{
	Super::BeginPlay();
}

void ABot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
