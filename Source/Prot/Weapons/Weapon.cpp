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

void AWeapon::Fire()
{	

	if (Role == ROLE_Authority)
	{
		ServerFire();
	}

	AActor* WeaponOwner = GetOwner(); // TODO: Define it in the ctor...
	if (WeaponOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;

		WeaponOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000.f);

		// Bullet spread...
		float HalfRad = FMath::DegreesToRadians(WeaponData.BulletSpread);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(WeaponOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		FVector TracerEndPoint = TraceEnd;
		EPhysicalSurface SurfaceType = SurfaceType_Default;
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			// Blocking hit! Process damage
			AActor* HitActor = Hit.GetActor();

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = WeaponData.BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, WeaponOwner->GetInstigatorController(), WeaponOwner, WeaponData.DamageType);

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;
		}

		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TracerEndPoint);

		if (Role == ROLE_Authority)
		{
			//HitScanTrace.TraceTo = TracerEndPoint;
			//HitScanTrace.SurfaceType = SurfaceType;
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

void AWeapon::ServerFire_Implementation()
{

}
bool AWeapon::ServerFire_Validate()
{
	return true;
}

void AWeapon::StartFire() {}
void AWeapon::StopFire() {}

void AWeapon::PlayFireEffects(FVector TraceEnd) {}
void AWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint) {}
