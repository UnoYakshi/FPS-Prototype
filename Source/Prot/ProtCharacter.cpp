// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ProtCharacter.h"
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
#include "GameFramework/SpringArmComponent.h"
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

	// Handle Weapon settings...
	WeaponAttachPoint = FName("weapon_socket_right");
	
	// Create weapon's skeletal mesh (TESTING ONLY)...
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSKMesh"));
	WeaponMesh->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponAttachPoint);

	// Health system...
	HealthComponent = CreateDefaultSubobject<UHealthActorComponent>(TEXT("Health"));
	HealthComponent->SetIsReplicated(true);

	CharText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DebugText"));
	CharText->SetRelativeLocation(FVector(0, 0, 100));
	CharText->SetupAttachment(GetRootComponent());

	// Default values for Interactive stuff...
	MaxUseDistance = 600.f;
	CurrentInteractive = nullptr;
}

void AProtCharacter::UpdateCharText()
{
	//Create a string that will display the health and bomb count values
	FString NewText = FString("Health: ") + FString::SanitizeFloat(HealthComponent->GetHealthValue());

	//Set the created string to the text render comp
	CharText->SetText(FText::FromString(NewText));
}

void AProtCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateCharText();

	WeaponMesh = CurrentWeapon->GetWeaponMesh();
	WeaponMesh->SetRelativeLocation(FVector::ZeroVector);
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

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AProtCharacter::TryStartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AProtCharacter::TryStopFire);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AProtCharacter::StartAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AProtCharacter::StopAim);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AProtCharacter::Reload);
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

bool AProtCharacter::CanFire() const
{
	return HealthComponent->IsAlive();
}

bool AProtCharacter::WeaponCanFire() const
{
	return CurrentWeapon->CanFire();
}

///
/// GRENADE & HP
///
float AProtCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	HealthComponent->DecreaseHealthValue(Damage);

	//Call the update text on the local client
	//OnRep_Health will be called in every other client so the character's text
	//will contain a text with the right values
	UpdateCharText();

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
	--GrenadeCount;
	UpdateCharText();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Instigator = this;
	SpawnParameters.Owner = GetController();

	GetWorld()->SpawnActor<AGrenade>(
		GrenadeClass,
		GetActorLocation() + GetActorForwardVector() * 200,
		GetActorRotation(),
		SpawnParameters);
}

void AProtCharacter::ServerSpawnGrenade_Implementation()
{
	SpawnGrenade();
}

bool AProtCharacter::ServerSpawnGrenade_Validate()
{
	return true;
}

///
/// FIREARMS
///
void AProtCharacter::TryStartFire()
{
	if (DEBUG)
	{
		switch (Role)
		{
		case ROLE_SimulatedProxy:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Char::LMB_On::SimProxy!"));
			break;
		case ROLE_AutonomousProxy:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Char::LMB_On::Client!"));
			break;
		case ROLE_Authority:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Char::LMB_On::Server!"));
			break;
		default:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Char::LMB_On::NANI!"));
			break;
		}
	}
	// TODO: Check if character CanFire()...

	JustFire();
}

void AProtCharacter::TryStopFire()
{
	if (DEBUG)
	{
		switch (Role)
		{
		case ROLE_SimulatedProxy:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Char::LMB_Off::SimProxy!"));
			break;
		case ROLE_AutonomousProxy:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Char::LMB_Off::Client!"));
			break;
		case ROLE_Authority:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Char::LMB_Off::Server!"));
			break;
		default:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Char::LMB_Off::NANI!"));
			break;
		}
	}

	JustFireEnd();
}

void AProtCharacter::JustFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		if (CurrentWeapon)
		{
			CurrentWeapon->StartFire();
		}
	}
}

void AProtCharacter::JustFireEnd()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
	}
}

void AProtCharacter::Reload()
{
	UE_LOG(LogTemp, Warning, TEXT("RELOAD"));

	UWorld* World = GetWorld();
	AMagazine* NewMag = World->SpawnActor<AMagazine>(
		MagClassToSpawn,
		GetActorLocation(),
		GetActorRotation()
		);
	if (NewMag)
	{
		UE_LOG(LogTemp, Warning, TEXT("CHAR::Reload"));
		NewMag->SetActorHiddenInGame(false);
		CurrentWeapon->ChangeMagazine(NewMag);
	}
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
	if (HealthComponent->IsAlive())
	{
		// calculate delta for this frame from the rate information
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AProtCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	if (HealthComponent->IsAlive())
	{
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}

void AProtCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && HealthComponent->IsAlive())
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
	if ((Controller != NULL) && (Value != 0.0f) && HealthComponent->IsAlive())
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

	// Unequip previous weapon if any...
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
	}

	CurrentWeapon = NewWeapon;

	// Equip a new one...
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
void AProtCharacter::OnRep_GrenadeCount()
{
	UpdateCharText();
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
