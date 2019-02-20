// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROT_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void UpdateRotation(float DeltaTime);

/* Custom input manipulation... */
public:
	FVector GetPlayerCameraInput();
	virtual bool IsCameraInputEnabled();
	virtual void InputCameraAddYaw(float Value);
	virtual void InputCameraAddPitch(float Value);
protected:
	bool bIsCameraInputEnabled = true;
	float ViewYawSpeed = 1.f;
	float ViewPitchSpeed = 1.f;
	bool bInvertCameraTurn = false;
	bool bInvertCameraUp = false;
	FVector PlayerCameraInput;
};
