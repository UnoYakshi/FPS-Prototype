// Fill out your copyright notice in the Description page of Project Settings.

#include "SwitcherObject.h"

bool ASwitcherObject::OnUse_Implementation(AActor* Initiator)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, FString::Printf(TEXT("SWITCH %s"), TEXT("OnUse")));
	return true;
}

bool ASwitcherObject::TurnOn_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, FString::Printf(TEXT("SWITCH %s"), TEXT("ON")));
	bIsUsed = true;
	return bIsUsed;
}

bool ASwitcherObject::TurnOff_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, FString::Printf(TEXT("SWITCH %s"), TEXT("OFF")));
	bIsUsed = false;
	return bIsUsed;
}

