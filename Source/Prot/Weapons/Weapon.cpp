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

void AWeapon::ServerFire()
{

}

// I don't know what is it and what UPROPERTY it should have (not defined on weapon.h
/*bool AWeapon::ServerFire_Validate() 
{
	return true;
}*/


void AWeapon::PlayFireEffects(FVector TraceEnd) {}
void AWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint) {}

void AWeapon::StartFire()
{

}

void AWeapon::Fire()
{
	//it was the one in ShooterExample but cleared it to delete FWeaponGeneralData and use FweaponData instead
}

void AWeapon::StopFire()
{
	LastFireTime = GetWorld()->GetTimeSeconds();
	//.............
}

void AWeapon::StartReload()
{
}

void AWeapon::StopReload()
{
}
