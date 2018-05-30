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

