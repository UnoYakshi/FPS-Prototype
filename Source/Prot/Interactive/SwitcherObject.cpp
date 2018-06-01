// Fill out your copyright notice in the Description page of Project Settings.

#include "SwitcherObject.h"

bool ASwitcherObject::OnUse_Implementation(AActor* Initiator)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("SWITCH %s"), "OnUse"));
	return true;

}

bool ASwitcherObject::TurnOn()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("SWITCH %s"), "ON"));
	return true;
}

bool ASwitcherObject::TurnOff()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("SWITCH %s"), "OFF"));
	return true;
}

