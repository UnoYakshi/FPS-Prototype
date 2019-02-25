// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FireInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFireInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROT_API IFireInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	bool bWantsToFire;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void TryStartFire();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void TryStopFire();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void JustFire();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void JustFireEnd();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Reload();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanFire();
};
