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
void AInteractiveObject::OnUse()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("INTERACTION %s"), "OnUse"));
}

void AInteractiveObject::OnFocusBegin()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("INTERACTION %s"), "OnFocusStart"));
}

void AInteractiveObject::OnFocusEnd()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("INTERACTION %s"), "OnFocusEnd"));
}

