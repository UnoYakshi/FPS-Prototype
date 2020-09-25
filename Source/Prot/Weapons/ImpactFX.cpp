// Fill out your copyright notice in the Description page of Project Settings.


#include "ImpactFX.h"

#include "Magazine.h"
#include "Engine/AutoDestroySubsystem.h"
#include "Engine/DecalActor.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

#include "Particles/ParticleSystemComponent.h"
#include "Components/DecalComponent.h"
#include "Components/AudioComponent.h"

#include "Runtime/Engine/Classes/PhysicalMaterials/PhysicalMaterial.h"


AImpactFX::AImpactFX()
{
	DecalComp = CreateDefaultSubobject<UDecalComponent>(L"Decal");
	// DecalComp->SetupAttachment(RootComponent);
	RootComponent = DecalComp;
	
	AudioComp = CreateDefaultSubobject<UAudioComponent>(L"Audio");
	AudioComp->SetupAttachment(RootComponent);
}

void AImpactFX::BeginPlay()
{
	Super::BeginPlay();
	
	for (const auto& Entry : Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("-- %d"), Entry.Key);
	}

	FVector CurrentActorLocation = GetActorLocation();
	
	UPhysicalMaterial* HitPhysMat = SurfaceHit.PhysMaterial.Get();
	EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitPhysMat);

	// Spawn VFX...
	UParticleSystem* ImpactFX = GetImpactFX(HitSurfaceType);
	if (ImpactFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, CurrentActorLocation, GetActorRotation());
	}

	// Spawn sound...
	USoundCue* ImpactSound = GetImpactSound(HitSurfaceType);
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, CurrentActorLocation);
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
	UMaterial* DecalMaterial = GetDecal(HitSurfaceType);
	if (DecalMaterial)
	{
		float DecalLifetime = 2.f;
		DecalComp->SetMaterial(0, DecalMaterial);
		DecalComp->SetLifeSpan(DecalLifetime);
		DecalComp->SetFadeOut(0.f, 10.f, true);
		DecalComp->DecalSize = FVector(32.f, 32.f, 32.f);

		// ADecalActor* NewDecal = GetWorld()->SpawnActor<ADecalActor>(CurrentActorLocation, FRotator());
		// if (NewDecal)
		// {
		// 	NewDecal->SetDecalMaterial(DecalMaterial);
		// 	NewDecal->SetLifeSpan(DecalLifetime);
		// 	NewDecal->GetDecal()->DecalSize = FVector(32.0f, 32.0f, 32.0f);
		// 	// m_previousActionDecal = NewDecal;
		// }
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No decal's spawned..."));
	}
}

void AImpactFX::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AImpactFX::PostInitializeComponents()
{
	Super::PostInitializeComponents();

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

UMaterial* AImpactFX::GetDecal(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	UMaterial* DecalMaterial = nullptr;

	if (Data.Contains(SurfaceType))
	{
		FMaterialImpactData CurrentSurfaceData = Data[SurfaceType];
		DecalMaterial = CurrentSurfaceData.DecalMaterial;
	}

	return DecalMaterial;
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
