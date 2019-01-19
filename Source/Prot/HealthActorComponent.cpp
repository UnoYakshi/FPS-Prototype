// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthActorComponent.h"

UHealthActorComponent::UHealthActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHealthActorComponent::BeginPlay()
{
	Super::BeginPlay();
	BringToLife();
}

void UHealthActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

//Checks if Death happened
void UHealthActorComponent::CheckDeath()
{
	if (!bIsDead && Health <= 0.f)
	{
		bIsDead = true;
		OnDeath.Broadcast(GetOwner());
	}
}

//Sets new current value of Health
void UHealthActorComponent::SetHealthValue(float NewHealthValue)
{
	if (NewHealthValue < 0.f)
	{
		Health = 0.f;
	}
	else if(NewHealthValue > MaxHealth)
	{
		Health = MaxHealth;
	}
	else
	{
		Health = NewHealthValue;
	}
	CheckDeath();
}
