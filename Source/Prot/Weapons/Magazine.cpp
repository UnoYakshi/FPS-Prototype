// Fill out your copyright notice in the Description page of Project Settings.

#include "Magazine.h"

// Sets default values
AMagazine::AMagazine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMagazine::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMagazine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMagazine::ConsumeAmmo(int32 NumToConsume)
{
	if (Data.CurrentAmmoNum - NumToConsume < 0)
	{
		return -(Data.CurrentAmmoNum - NumToConsume);
	}

	Data.CurrentAmmoNum -= NumToConsume;
}

void AMagazine::AddAmmo(int32 NumToAdd)
{
	Data.CurrentAmmoNum = FMath::Clamp(Data.CurrentAmmoNum + NumToAdd, 0, Data.MaxCapacity);
}
