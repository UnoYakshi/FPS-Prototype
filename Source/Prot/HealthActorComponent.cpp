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
	CheckDeath();
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
