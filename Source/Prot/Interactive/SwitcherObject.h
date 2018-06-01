// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactive/InteractiveObject.h"
#include "SwitcherObject.generated.h"

/**
 * 
 */
UCLASS()
class PROT_API ASwitcherObject : public AInteractiveObject
{
	GENERATED_BODY()

public:
	virtual void OnUse() override;
	virtual void TurnOn();
	virtual void TurnOff();
	
	
};
