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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool OnUse(AActor* Initiator);
	bool OnUse_Implementation(AActor* Initiator);

	//UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	//bool OnStopUsing(AActor* Initiator) override;

	bool TurnOn();
	bool TurnOff();
	
	
};
