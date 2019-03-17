// Fill out your copyright notice in the Description page of Project Settings.

#include "Bot.h"
#include "Camera/CameraComponent.h"
#include "Weapons/WeaponComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AI/BotAIC.h"

ABot::ABot()
{
}

void ABot::BeginPlay()
{
	Super::BeginPlay();
}

void ABot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ABotAIC* BotController = Cast<ABotAIC>(Controller);
	UBlackboardComponent* BlackboardComponent = BotController->GetBlackboardComponent();
	if (BlackboardComponent)
	{
		APawn* TargetPawn = Cast<APawn>(BlackboardComponent->GetValueAsObject("TargetPlayer"));
		if (TargetPawn)
		{
			CameraLookAt(TargetPawn);
		}
	}
}

void ABot::CameraLookAt(AActor* Actor)
{
	FRotator rotator = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Actor->GetActorLocation());
	FPPCamera->SetWorldRotation(rotator);
}
