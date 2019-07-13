// Fill out your copyright notice in the Description page of Project Settings.


#include "ImpactFX.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/PhysicalMaterials/PhysicalMaterial.h"

//const TMap<FString, int32> AImpactFX::TestVar = {
//	{"5", 5},
//	{"3", 3}
//};

AImpactFX::AImpactFX()
{
	bAutoDestroyWhenFinished = true;

	/*for (const auto& Entry : TestVar)
	{
		UE_LOG(LogTemp, Warning, TEXT("-- %d"), Entry.Value);
	}*/
}

void AImpactFX::BeginPlay()
{
	Super::BeginPlay();
	
	for (const auto& Entry : Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("-- %d"), Entry.Key);
	}
}

void AImpactFX::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AImpactFX::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UPhysicalMaterial* HitPhysMat = SurfaceHit.PhysMaterial.Get();
	EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitPhysMat);

	// show particles
	UParticleSystem* ImpactFX = GetImpactFX(HitSurfaceType);
	if (ImpactFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, GetActorLocation(), GetActorRotation());
	}

	// play sound
	USoundCue* ImpactSound = GetImpactSound(HitSurfaceType);
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	/*if (DefaultDecal.DecalMaterial)
	{
		FRotator RandomDecalRotation = SurfaceHit.ImpactNormal.Rotation();
		RandomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);

		UGameplayStatics::SpawnDecalAttached(DefaultDecal.DecalMaterial, FVector(1.0f, DefaultDecal.DecalSize, DefaultDecal.DecalSize),
			SurfaceHit.Component.Get(), SurfaceHit.BoneName,
			SurfaceHit.ImpactPoint, RandomDecalRotation, EAttachLocation::KeepWorldPosition,
			DefaultDecal.LifeSpan);
	}*/
}

UParticleSystem* AImpactFX::GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	UParticleSystem* ImpactFX = nullptr;

	if (Data.Contains(SurfaceType))
	{
		FMaterialImpactData CurrentSurfaceData = Data[SurfaceType];
		ImpactFX = CurrentSurfaceData.ImpactFX;
	}

	return ImpactFX;
}

USoundCue* AImpactFX::GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	USoundCue* ImpactSound = nullptr;

	if (Data.Contains(SurfaceType))
	{
		FMaterialImpactData CurrentSurfaceData = Data[SurfaceType];
		ImpactSound = CurrentSurfaceData.ImpactSound;
	}

	return ImpactSound;
}
