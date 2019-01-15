// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractiveObject.h"


AInteractiveObject::AInteractiveObject()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsUsed = false;

}

void AInteractiveObject::BeginPlay()
{
	Super::BeginPlay();
	
}

void AInteractiveObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Interactive...
bool AInteractiveObject::OnUse_Implementation(AActor* Initiator)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, FString::Printf(TEXT("INTERACTION %s"), TEXT("OnUse")));
	bIsUsed = true;
	return bIsUsed;
}

bool AInteractiveObject::OnStopUsing_Implementation(AActor* Initiator)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, FString::Printf(TEXT("INTERACTION %s"), TEXT("OnStopUsing")));
	bIsUsed = false;
	return bIsUsed;
}

bool AInteractiveObject::OnFocusBegin_Implementation()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, FString::Printf(TEXT("INTERACTION %s"), "OnFocusStart"));
	return true;
}

bool AInteractiveObject::OnFocusEnd_Implementation()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, FString::Printf(TEXT("INTERACTION %s"), TEXT("OnFocusEnd")));
	return true;
}

