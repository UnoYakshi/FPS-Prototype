// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractiveObject.generated.h"

/** Adding an ability to assign delegates that will be called
 *  when the object is used */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUseSignature, AActor*, Initiator);

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bIsUsed;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool OnUse(AActor* Initiator);
	bool OnUse_Implementation(AActor* Initiator);

	/** Called when the object is used */
	UPROPERTY(BlueprintAssignable, Category = "Game|Interaction")
	FUseSignature OnUseWithDelegates;

	/** Called when the object is stopped being used */
	UPROPERTY(BlueprintAssignable, Category = "Game|Interaction")
	FUseSignature OnStopUsingWithDelegates;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool OnStopUsing(AActor* Initiator);
	bool OnStopUsing_Implementation(AActor* Initiator);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool OnFocusBegin();
	bool OnFocusBegin_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool OnFocusEnd();
	bool OnFocusEnd_Implementation();
};
