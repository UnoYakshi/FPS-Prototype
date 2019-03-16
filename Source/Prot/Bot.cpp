// Fill out your copyright notice in the Description page of Project Settings.

#include "Bot.h"
#include "Camera/CameraComponent.h"
#include "Weapons/WeaponComponent.h"
#include "Kismet/KismetMathLibrary.h"

ABot::ABot()
{
	FPPCamera->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void ABot::BeginPlay()
{
	Super::BeginPlay();
}

void ABot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABot::CameraLookAt(AActor* Actor)
{
	FRotator rotator = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Actor->GetActorLocation());
	FPPCamera->SetWorldRotation(rotator);
}