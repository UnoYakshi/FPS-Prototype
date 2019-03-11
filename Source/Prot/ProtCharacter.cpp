// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ProtCharacter.h"
#include "Prot.h"
#include "MyPlayerController.h"
#include "Weapons/Weapon.h"
#include "Interactive/InteractiveObject.h"

#include "Engine.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"


//////////////////////////////////////////////////////////////////////////
// AProtCharacter

AProtCharacter::AProtCharacter()
{
	// Enable ticking...
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;

	// Networking...
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bNetUseOwnerRelevancy = true;

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
	FPPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPPCamera"));
	FPPCamera->SetupAttachment(GetMesh(), "eyes");
	FPPCamera->bUsePawnControlRotation = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	// Health system...
	HealthComponent = CreateDefaultSubobject<UHealthActorComponent>(TEXT("Health"));
	HealthComponent->SetIsReplicated(true);

	// Create weapon component...
	CurrentWeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComp"));
	CurrentWeaponComp->SetIsReplicated(true);
	CurrentWeaponComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CurrentWeaponComp->WeaponAttachPoint);

	// Default values for Interactive stuff...
	MaxUseDistance = 600.f;
	CurrentInteractive = nullptr;
}

void AProtCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CurrentWeapon && CurrentWeapon->GetWeaponMesh())
	{
		CurrentWeapon->GetWeaponMesh()->SetRelativeLocation(FVector::ZeroVector);
		//CurrentWeapon->OnEquip(nullptr);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input
void AProtCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//PreUpdateCamera(DeltaTime);
}

void AProtCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AProtCharacter::TryStartFire);
	//PlayerInputComponent->BindAction("Fire", IE_Released, this, &AProtCharacter::TryStopFire);
	//PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AProtCharacter::StartAim);
	//PlayerInputComponent->BindAction("Aim", IE_Released, this, &AProtCharacter::StopAim);
	//PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AProtCharacter::Reload);
	PlayerInputComponent->BindAction("ThrowBomb", IE_Pressed, this, &AProtCharacter::AttempToSpawnGrenade);

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

	FCollisionQueryParams TraceParams(FName(TEXT("InteractiveRaytrace")), true, this);
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
	if (Controller && Controller->IsLocalController())
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
	return HealthComponent->IsAlive();
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
/// GRENADE & HP
///
float AProtCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	HealthComponent->DecreaseHealthValue(Damage);

	return HealthComponent->GetHealthValue();
}

void AProtCharacter::ServerTakeDamage_Implementation(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

bool AProtCharacter::ServerTakeDamage_Validate(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	//Assume that everything is ok without any further checks and return true
	return true;
}

void AProtCharacter::AttempToSpawnGrenade()
{
	if (HasGrenades())
	{
		if (Role < ROLE_Authority)
		{
			ServerSpawnGrenade();
		}
		else
		{
			SpawnGrenade();
		}
	}
}

void AProtCharacter::SpawnGrenade()
{
	/*UGameplayStatics::SuggestProjectileVelocity(
		this,
		FVector(100.f, 0.f, 0.f),
		GetActorLocation(),
		FVector(0.f, 0.f, 0.f),
		400.f,
		true,
		3.f,
		0,
		ESuggestProjVelocityTraceOption::TraceFullPath,
		FCollisionResponseParams::DefaultResponseParam,
		TArray<AActor*>(),
		true);
	*/
	--GrenadeCount;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Instigator = this;
	SpawnParameters.Owner = GetController();

	// Find "crosshair" forward vector...
	FVector camLoc;
	FRotator camRot;
	Controller->GetPlayerViewPoint(camLoc, camRot);
	const FVector Direction = camRot.Vector();

	AGrenade* NewGrenade = GetWorld()->SpawnActor<AGrenade>(
		GrenadeClass,
		camLoc,
		GetActorRotation(),
		SpawnParameters);
	if (NewGrenade)
	{
		// Add speed...
		NewGrenade->FireInDirection(Direction * 12.f);
	}
}

void AProtCharacter::ServerSpawnGrenade_Implementation()
{
	SpawnGrenade();
}

bool AProtCharacter::ServerSpawnGrenade_Validate()
{
	return true;
}


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
	if (Role < ROLE_Authority)
	{
		FRotator ControlRot = GetControlRotation();
		FRotator ActorRot = GetActorRotation();
		FRotator Res = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
		TurnAtRateServer(Rate);
	}

	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AProtCharacter::TurnAtRateServer_Implementation(float Rate)
{
	CurTurnRate = Rate;
}

bool AProtCharacter::TurnAtRateServer_Validate(float Rate)
{
	return true;
}

void AProtCharacter::LookUpAtRate(float Rate)
{
	if (Role < ROLE_Authority)
	{
		FRotator ControlRot = GetControlRotation();
		FRotator ActorRot = GetActorRotation();
		FRotator Res = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
		LookUpAtRateServer(Res.Pitch);
	}

	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AProtCharacter::LookUpAtRateServer_Implementation(float Rate)
{
	CurLookUpRate = Rate;
}

bool AProtCharacter::LookUpAtRateServer_Validate(float Rate)
{
	return true;
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
	if ((Controller != NULL) && (Value != 0.0f))
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
	if (!FPPCamera || !PC || HealthComponent->IsDead())
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

	CameraTreshold = 20.f;
	RecoilOffset = 10.f;
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
			//SetCurrentWeapon(Weapon, CurrentWeapon);
		}
		else
		{
			//ServerEquipWeapon(Weapon);
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



/////////////////////////////////////////////
// REPLICATION

void AProtCharacter::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	//SetCurrentWeapon(CurrentWeapon, LastWeapon);
}
void AProtCharacter::OnRep_GrenadeCount()
{
}
void AProtCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/// only to local owner: weapon change requests are locally instigated, other clients don't need it
	///DOREPLIFETIME_CONDITION(AShooterCharacter, Inventory, COND_OwnerOnly);

	DOREPLIFETIME(AProtCharacter, CurrentWeapon);
	DOREPLIFETIME(AProtCharacter, CameraLocalRotation);
}

/////////////////////////////////////////////
// UTILITY

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
