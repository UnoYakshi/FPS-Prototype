// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractiveObject.h"


AInteractiveObject::AInteractiveObject()
{
	PrimaryActorTick.bCanEverTick = true;

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
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("INTERACTION %s"), "OnUse"));
	return true;
}

bool AInteractiveObject::OnStopUsing_Implementation(AActor* Initiator)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("INTERACTION %s"), "OnStopUsing"));
	return true;
}

bool AInteractiveObject::OnFocusBegin_Implementation()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("INTERACTION %s"), "OnFocusStart"));
	return true;
}

bool AInteractiveObject::OnFocusEnd_Implementation()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("INTERACTION %s"), "OnFocusEnd"));
	return true;
}

