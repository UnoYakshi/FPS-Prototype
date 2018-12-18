// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"
#include "Prot.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	_T("PROT.DebugWeapons"),
	DebugWeaponDrawing,
	_T("Draw Debug Lines for Weapons"),
	ECVF_Cheat);


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	SKMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = SKMeshComp;

	MuzzleSocketName = FName("Muzzle");
	TracerTargetName = FName("Target");

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::ServerFire_Implementation()
{

}

bool AWeapon::ServerFire_Validate() 
{
	return true;
}


void AWeapon::PlayFireEffects(FVector TraceEnd) {}
void AWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint) {}

bool AWeapon::CanReload()
{
	return true;
}
bool AWeapon::CanFire()
{
	return true;
}
void AWeapon::StartFire()
{

	if (CurrentMagazine->Data.CurrentAmmoNum <= 0)//No Ammo
	{
		//TODO : play sound here
		return;
	}
	WeaponData.bIsFiring = true;
	Fire();
}

void AWeapon::Fire()
{
	if (Carrier && CanFire() && CurrentMagazine->Data.CurrentAmmoNum > 0)
	{
		///TODO : Server Stuff
		///TODO : Add Firing System
		UseAmmo();
	}
	else if (CanReload())
	{
		StartReload();
	}
	else
	{
		//Play Out of Ammo Sound
	}
	LastFireTime = GetWorld()->GetTimeSeconds();
}

void AWeapon::StopFire()
{
	WeaponData.bIsFiring = false;
	//TODO add stop animation

}

void AWeapon::StartReload()
{
}

void AWeapon::StopReload()
{
}

void AWeapon::UseAmmo()
{
	CurrentMagazine->ConsumeAmmo(1);
}
void AWeapon::OnEquip(const AWeapon* LastWeapon)
{

}
void AWeapon::OnEquipFinished()
{
}
void AWeapon::ServerStartFire()
{
}
void AWeapon::ServerStopFire()
{
}
void AWeapon::ServerStartReload()
{
}
void AWeapon::ServerStopReload()
{
}
void AWeapon::ServerHandleFiring()
{
}
void AWeapon::ClientHandleFiring()
{
}
void AWeapon::ClientFire()
{
}
void AWeapon::Destroyed()
{
	Super::Destroyed();

	//StopSimulatingWeaponFire();
}