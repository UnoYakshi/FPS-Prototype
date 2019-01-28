// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"


UHealthActorComponent::UHealthActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);

}

void UHealthActorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHealthActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UHealthActorComponent::CheckDeath()
{
	if (!bIsDead && Health <= 0.f)
	{
		bIsDead = true;
		OnDeath.Broadcast(GetOwner());
		Die();
	}
}

void UHealthActorComponent::SetHealthValue(const float NewHealthValue)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,
		FString::Printf(TEXT("B4 HP: %f"), Health)
	);

	if (GetOwner()->Role < ROLE_Authority)
	{
		Server_SetHealthValue(NewHealthValue);
	}

	if (NewHealthValue < 0.f)
	{
		Health = 0.f;
	}
	else if (NewHealthValue >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else
	{
		Health = NewHealthValue;
	}

	CheckDeath();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,
		FString::Printf(TEXT("AFTER HP: %f"), Health)
	);

}

void UHealthActorComponent::Server_SetHealthValue_Implementation(const float NewHealthValue)
{
	SetHealthValue(NewHealthValue);
}
bool UHealthActorComponent::Server_SetHealthValue_Validate(const float NewHealthValue)
{
	return true;
}

void UHealthActorComponent::IncreaseHealthValue(const float IncreaseValue)
{
	SetHealthValue(Health + IncreaseValue);
}

void UHealthActorComponent::DecreaseHealthValue(float DecreaseValue)
{
	SetHealthValue(Health - DecreaseValue);
}

void UHealthActorComponent::Die()
{
	switch (GetOwner()->Role)
	{
	case ROLE_SimulatedProxy:
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("I am a dead SIMULATED_PROXY!"));
		break;
	case ROLE_AutonomousProxy:
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("I am a dead AUTONOMOUS_PROXY!"));
		break;
	case ROLE_Authority:
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("I am a dead SERVER!"));
		break;
	default:
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("I am just dead!"));
		break;
	}
}

void UHealthActorComponent::BringToLife(float NewHealthValue)
{
	bIsDead = false;
	SetHealthValue(NewHealthValue);	
}

void UHealthActorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthActorComponent, Health);
	DOREPLIFETIME(UHealthActorComponent, bIsDead);
}

void UHealthActorComponent::OnRep_Health()
{
	CheckDeath();
}
void UHealthActorComponent::OnRep_IsDead()
{
	if (bIsDead)
	{
		Die();
		// TODO: Call GetOwner()->OnDeath() or something...
	}
}