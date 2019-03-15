// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponComponent.h"

#include "Prot.h"
#include "Projectile.h"
//#include "MyPlayerController.h"
#include "ProtCharacter.h"

//#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"


UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	WeaponAttachPoint = FName("weapon_socket_right");
	IronsightSocketName = FName("Ironsight");

	//MyPawn = Cast<APawn>(GetOwner());
}

void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsAiming && CurrentWeapon)
	{
		UCameraComponent* Camera = GetOwner()->FindComponentByClass<UCameraComponent>();
		FMinimalViewInfo CamData;
		Camera->GetCameraView(DeltaTime, CamData);
		
		// Set weapon location aligned with the ironsight...
		FVector CameraLoc = CamData.Location;
		FVector NewWeaponLoc = CameraLoc + (CurrentWeapon->GetActorLocation() - GetIronsightSocketLocaction());
		CurrentWeapon->SetActorLocation(NewWeaponLoc);

		// Set weapon rotation aligned with the camera...
		FRotator CameraRot = CamData.Rotation;
		CurrentWeapon->SetActorRotation(CameraRot);
	}
}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// "Automatically" bind inputs...
	UInputComponent* OwnerInput = GetOwner()->InputComponent;
	if (OwnerInput)
	{
		OwnerInput->BindAction("Fire", IE_Pressed, this, &UWeaponComponent::TryStartFire);
		OwnerInput->BindAction("Fire", IE_Released, this, &UWeaponComponent::TryStopFire);
		OwnerInput->BindAction("Aim", IE_Pressed, this, &UWeaponComponent::StartAim);
		OwnerInput->BindAction("Aim", IE_Released, this, &UWeaponComponent::StopAim);
		OwnerInput->BindAction("Reload", IE_Pressed, this, &UWeaponComponent::Reload);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginPlay::NoPlayerInputComponent"));
	}
	
	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorRelativeLocation(FVector::ZeroVector);
		CurrentWeapon->OnEquip(nullptr);
	}
}

void UWeaponComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if (CurrentWeapon)
	{
		CurrentWeapon->StopSimulatingWeaponFire();
	}
}

bool UWeaponComponent::CanFire() const
{
	UHealthActorComponent* HealthComponent = GetOwner()->FindComponentByClass<UHealthActorComponent>();
	if (HealthComponent)
	{
		return HealthComponent->IsAlive();
	}
	return true;
}

bool UWeaponComponent::WeaponCanFire() const
{
	return CurrentWeapon->CanFire();
}

///
/// FIREARMS
///
void UWeaponComponent::TryStartFire()
{
	if (DEBUG)
	{
		switch (GetOwnerRole())
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

void UWeaponComponent::TryStopFire()
{
	if (DEBUG)
	{
		switch (GetOwnerRole())
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

void UWeaponComponent::JustFire()
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

void UWeaponComponent::JustFireEnd()
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


void UWeaponComponent::Reload()
{
	UWorld* World = GetWorld();
	AMagazine* NewMag = World->SpawnActor<AMagazine>(
		MagClassToSpawn,
		GetOwner()->GetActorLocation(),
		GetOwner()->GetActorRotation()
		);
	if (NewMag)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponComponent::Reload"));
		NewMag->SetActorHiddenInGame(false);
		CurrentWeapon->ChangeMagazine(NewMag);
	}
}

bool UWeaponComponent::CanReload()
{
	return true;
}


void UWeaponComponent::StartAim_Implementation()
{
	bIsAiming = true;
}

bool UWeaponComponent::StartAim_Validate()
{
	return true;
}

void UWeaponComponent::StopAim_Implementation()
{
	bIsAiming = false;
	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorRelativeLocation(FVector::ZeroVector);
		CurrentWeapon->SetActorRelativeRotation(FRotator::ZeroRotator);
	}
}

bool UWeaponComponent::StopAim_Validate()
{
	return true;
}

FVector UWeaponComponent::GetIronsightSocketLocaction() const
{
	return (CurrentWeapon && CurrentWeapon->GetWeaponMesh()) 
	? CurrentWeapon->GetWeaponMesh()->GetSocketLocation(IronsightSocketName) 
	: FVector::ZeroVector;
}


void UWeaponComponent::SetCurrentWeapon(AWeapon* NewWeapon, AWeapon* LastWeapon)
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
		NewWeapon->SetOwningPawn(Cast<ACharacter>(GetOwner()));	// Make sure weapon's MyPawn is pointing back to us. 
		// During replication, we can't guarantee APawn::CurrentWeapon will rep after AWeapon::MyPawn!
		
		NewWeapon->OnEquip(LastWeapon);
	}
}

void UWeaponComponent::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon)
	{
		if (GetOwnerRole() == ROLE_Authority)
		{
			SetCurrentWeapon(Weapon, CurrentWeapon);
		}
		else
		{
			ServerEquipWeapon(Weapon);
		}
	}
}

void UWeaponComponent::ServerEquipWeapon_Implementation(AWeapon* Weapon)
{
	EquipWeapon(Weapon);
}

bool UWeaponComponent::ServerEquipWeapon_Validate(AWeapon* Weapon)
{
	return true;
}

void UWeaponComponent::OnRep_CurrentWeapon(class AWeapon* LastWeapon)
{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponComponent::CurrentWeaponChange"));
}


void UWeaponComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponComponent, CurrentWeapon);
}