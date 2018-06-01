// Fill out your copyright notice in the Description page of Project Settings.

#include "SwitcherObject.h"

void ASwitcherObject::OnUse()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("SWITCH %s"), "OnUse"));
}

void ASwitcherObject::TurnOn()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("SWITCH %s"), "ON"));
}

void ASwitcherObject::TurnOff()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(_T("SWITCH %s"), "OFF"));
}

