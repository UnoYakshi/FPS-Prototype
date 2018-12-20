// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ProtCharacter.h"
#include "MyPlayerController.h"
#include "Weapons/Weapon.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Interactive/InteractiveObject.h"
#include "Kismet/KismetMathLibrary.h"


//////////////////////////////////////////////////////////////////////////
// AProtCharacter

AProtCharacter::AProtCharacter()
{
	// Enable ticking...
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create and set-up FPP camera...
	// TODO: Make blendspace to make pitch more seamless...
	FPPCamera = CreateDefaultSubobject<UCameraComponent>(_T("FPPCamera"));
	FPPCamera->SetupAttachment(GetMesh(), "eyes");
	FPPCamera->bUsePawnControlRotation = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create weapon's skeletal mesh...
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSKMesh"));
	if (WeaponMesh)
	{
		WeaponMesh->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("weapon_r"));
	}

	// Default values for Interactive stuff...
	MaxUseDistance = 600.f;
	CurrentInteractive = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Input
void AProtCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PreUpdateCamera(DeltaTime);
}

void AProtCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AProtCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AProtCharacter::StopFire);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AProtCharacter::StartAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AProtCharacter::StopAim);

	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &AProtCharacter::Use);
	PlayerInputComponent->BindAction("Use", IE_Released, this, &AProtCharacter::StopUsing);


	//PlayerInputComponent->BindAxis("MoveForward", PC, &AMyPlayerController::InputMovementFront);
	//PlayerInputComponent->BindAxis("MoveRight", PC, &AMyPlayerController::InputMovementSide);
	PlayerInputComponent->BindAxis("CameraRotationVertical", PC, &AMyPlayerController::InputCameraAddPitch);
	PlayerInputComponent->BindAxis("CameraRotationHorizontal", PC, &AMyPlayerController::InputCameraAddYaw);
	PlayerInputComponent->BindAxis("MoveForward", this, &AProtCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProtCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AProtCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProtCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AProtCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AProtCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AProtCharacter::OnResetVR);
}

FRotator AProtCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}


AInteractiveObject* AProtCharacter::GetInteractiveInView()
{
	FVector camLoc;
	FRotator camRot;

	if (Controller == nullptr)
	{
		return nullptr;
	}

	Controller->GetPlayerViewPoint(camLoc, camRot);
	const FVector StartTrace = camLoc;
	const FVector direction = camRot.Vector();
	const FVector EndTrace = StartTrace + (direction * MaxUseDistance);

	FCollisionQueryParams TraceParams(FName(_T("InteractiveRaytrace")), true, this);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_Visibility, TraceParams);
	DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor(255, 255, 255), false, 1);

	return Cast< AInteractiveObject >(Hit.GetActor());
}

/*
Runs on Server. Perform "OnUsed" on currently viewed UsableActor if implemented.
*/
void AProtCharacter::Use_Implementation()
{
	if (this->Controller && this->Controller->IsLocalController())
	{
		AInteractiveObject* Usable = this->GetInteractiveInView();
		if (Usable)
		{
			CurrentInteractive = Usable;
			Usable->OnUse(this);
		}
		else
		{
			CurrentInteractive = nullptr;
		}
	}

}

bool AProtCharacter::Use_Validate()
{
	return true;
}

void AProtCharacter::StopUsing_Implementation()
{
	if (CurrentInteractive)
	{
		CurrentInteractive->OnStopUsing(this);
		CurrentInteractive = nullptr;
	}
}

bool AProtCharacter::StopUsing_Validate()
{
	return true;
}

///
/// FIREARMS
///
void AProtCharacter::StartFire_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Fire at will!"));
	CurrentWeapon->StartFire();
}

bool AProtCharacter::StartFire_Validate()
{
	return true;
}

void AProtCharacter::StopFire_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Halt your fire!"));
	CurrentWeapon->StopFire();
}

bool AProtCharacter::StopFire_Validate()
{
	return true;
}

void AProtCharacter::StartAim_Implementation()
{

}

bool AProtCharacter::StartAim_Validate()
{
	return true;
}

void AProtCharacter::StopAim_Implementation()
{

}

bool AProtCharacter::StopAim_Validate()
{
	return true;
}
///

void AProtCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AProtCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AProtCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AProtCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AProtCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AProtCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AProtCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void AProtCharacter::PreUpdateCamera(float DeltaTime)
{
	if (!FPPCamera || !PC)
	{
		return;
	}
	//-------------------------------------------------------
	// Compute the rotation for Mesh AimOffset...
	//-------------------------------------------------------
	FRotator ControllerRotation = PC->GetControlRotation();
	FRotator NewRotation = ControllerRotation;

	// Get current controller rotation and process it to match the Character
	NewRotation.Yaw = CameraProcessYaw(ControllerRotation.Yaw);
	NewRotation.Pitch = CameraProcessPitch(ControllerRotation.Pitch + RecoilOffset);
	NewRotation.Normalize();

	// Clamp new rotation
	NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch, -90.0f + CameraTreshold, 90.0f - CameraTreshold);
	NewRotation.Yaw = FMath::Clamp(NewRotation.Yaw, -91.0f, 91.0f);

	// Will be retrieved by AnimBlueprint...
	CameraLocalRotation = NewRotation;
}

float AProtCharacter::CameraProcessPitch(float Input)
{
	//Recenter value
	if (Input > 269.99f)
	{
		Input -= 270.0f;
		Input = 90.0f - Input;
		Input *= -1.0f;
	}

	return Input;
}

void AProtCharacter::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon)
	{
		if (Role == ROLE_Authority)
		{
			SetCurrentWeapon(Weapon, CurrentWeapon);
		}
		else
		{
			ServerEquipWeapon(Weapon);
		}
	}
}

bool AProtCharacter::ServerEquipWeapon_Validate(AWeapon* Weapon)
{
	return true;
}

void AProtCharacter::ServerEquipWeapon_Implementation(AWeapon* Weapon)
{
	EquipWeapon(Weapon);
}

void AProtCharacter::SetCurrentWeapon(AWeapon* NewWeapon, AWeapon* LastWeapon)
{
	AWeapon* LocalLastWeapon = nullptr;

	if (LastWeapon)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	// unequip previous
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
	}

	CurrentWeapon = NewWeapon;

	// equip new one
	if (NewWeapon)
	{
		NewWeapon->SetOwningPawn(this);	// Make sure weapon's MyPawn is pointing back to us. 
		// During replication, we can't guarantee APawn::CurrentWeapon will rep after AWeapon::MyPawn!
		
		NewWeapon->OnEquip(LastWeapon);
	}
}

/////////////////////////////////////////////
// REPLICATION

void AProtCharacter::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}
void AProtCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


}

/////////////////////////////////////////////
// UTILITY
FName AProtCharacter::GetWeaponAttachPoint() const
{
	return WeaponAttachPoint;
}

float AProtCharacter::CameraProcessYaw(float Input)
{
	// Get direction vectors from Character and PC...
	FVector ActorDir = GetActorRotation().Vector();
	FVector PCDir = FRotator(0.f, Input, 0.f).Vector();

	// Compute the Angle difference between the two direction
	float Angle = FMath::Acos(FVector::DotProduct(ActorDir, PCDir));
	Angle = FMath::RadiansToDegrees(Angle);

	// Find on which side is the angle difference (left or right)
	FRotator Temp = GetActorRotation() - FRotator(0.f, 90.f, 0.f);
	FVector Direction3 = Temp.Vector();

	float Dot = FVector::DotProduct(Direction3, PCDir);

	// Invert angle to switch side
	if (Dot > 0.f)
	{
		Angle *= -1;
	}

	return Angle;
}

