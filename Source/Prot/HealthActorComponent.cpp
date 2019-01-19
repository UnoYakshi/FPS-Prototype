// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthActorComponent.h"

UHealthActorComponent::UHealthActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	BringToLife();
}

void UHealthActorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHealthActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CheckDeath();
}

//Checks if Death happened
void UHealthActorComponent::CheckDeath()
{
	if (!isDead && Health <= 0.f)
	{
		isDead = true;
		OnDeath.Broadcast(GetOwner());
	}
}
