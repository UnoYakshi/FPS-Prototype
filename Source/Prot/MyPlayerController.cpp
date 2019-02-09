// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"


void AMyPlayerController::UpdateRotation(float DeltaTime)
{
	if (!IsCameraInputEnabled())
		return;

	float Time = DeltaTime * (1 / GetActorTimeDilation());
	FRotator DeltaRot(0, 0, 0);

	DeltaRot.Yaw = GetPlayerCameraInput().X * (ViewYawSpeed * Time);
	DeltaRot.Pitch = GetPlayerCameraInput().Y * (ViewPitchSpeed * Time);
	DeltaRot.Roll = 0.0f;

	RotationInput = DeltaRot;

	Super::UpdateRotation(DeltaTime);
}

bool AMyPlayerController::IsCameraInputEnabled()
{
	return bIsCameraInputEnabled;
}

void AMyPlayerController::InputCameraAddYaw(float Value)
{
	PlayerCameraInput.X = bInvertCameraTurn ? Value * -1.f : Value;
}

void AMyPlayerController::InputCameraAddPitch(float Value)
{
	PlayerCameraInput.Y = bInvertCameraUp ? Value * -1.f : Value;
}

FVector AMyPlayerController::GetPlayerCameraInput()
{
	return PlayerCameraInput;
}

