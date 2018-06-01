// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractiveObject.generated.h"

UCLASS()
class PROT_API AInteractiveObject : public AActor
{
	GENERATED_BODY()
	
public:	
	AInteractiveObject();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/** Interactive "interface"... */
	virtual void OnUse();
	virtual void OnStopUsing();

	virtual void OnFocusBegin();
	virtual void OnFocusEnd();
	
};
