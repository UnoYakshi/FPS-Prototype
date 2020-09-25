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
	DecalComp->SetIsReplicated(true);
	RootComponent = DecalComp;
	
	AudioComp = CreateDefaultSubobject<UAudioComponent>(L"Audio");
	AudioComp->SetIsReplicated(true);
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
		UGameplayStatics::SpawnEmitterAtLocation(
			this,
			ImpactFX,
			CurrentActorLocation,
			FRotator::ZeroRotator,
			FVector(1),
			true,
			EPSCPoolMethod::AutoRelease,
			true
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No VFX's spawned..."));
	}

	// Spawn decal...
	UMaterial* DecalMaterial = GetDecal(HitSurfaceType);
	if (DecalMaterial && DecalComp)
	{
		float DecalLifetime = 2.f;
		DecalComp->SetMaterial(0, DecalMaterial);
		DecalComp->SetLifeSpan(DecalLifetime);
		DecalComp->SetFadeOut(0.f, 10.f, true);
		DecalComp->DecalSize = FVector(32.f, 32.f, 32.f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No decal's spawned..."));
	}

	// Spawn sound...
	USoundCue* ImpactSound = GetImpactSound(HitSurfaceType);
	if (ImpactSound && AudioComp)
	{
		// UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, CurrentActorLocation);

		AudioComp->SetSound(ImpactSound);
		// AudioComp->ActiveCount = 1;
		AudioComp->bAutoDestroy = true;
		if (!AudioComp->IsActive()) 
		{
			AudioComp->Activate(true);
		}
		// default pitch value 1.0f
		// modify the pitch to create variance by grabbing a random float between 1.0 and 1.3
		AudioComp->SetPitchMultiplier(FMath::RandRange(1.0f, 1.3f));
		AudioComp->SetVolumeMultiplier(3.f);
		FSoundAttenuationSettings NewSettings = FSoundAttenuationSettings();
		NewSettings.bSpatialize = true;
		NewSettings.BinauralRadius = 150.f;
		NewSettings.FalloffDistance = 500.f;
		NewSettings.SpatializationAlgorithm = ESoundSpatializationAlgorithm::SPATIALIZATION_HRTF;
		AudioComp->AdjustAttenuation(NewSettings);

		// play the sound
		AudioComp->Play(0.f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No SFX's spawned..."));
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
