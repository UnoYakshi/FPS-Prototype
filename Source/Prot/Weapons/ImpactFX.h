// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Components/DecalComponent.h"
#include "Materials/Material.h"


// #include "ImpactSpawner.h"

#include "ImpactFX.generated.h"


#define SURFACE_Default		SurfaceType_Default
#define SURFACE_Concrete	SurfaceType1
#define SURFACE_Dirt		SurfaceType2
#define SURFACE_Water		SurfaceType3
#define SURFACE_Metal		SurfaceType4
#define SURFACE_Wood		SurfaceType5
#define SURFACE_Grass		SurfaceType6
#define SURFACE_Glass		SurfaceType7

#define SURFACE_Flesh		SurfaceType8
#define SURFACE_Bone		SurfaceType9
#define SURFACE_Monster		SurfaceType10

#define SURFACE_Snow		SurfaceType11
#define SURFACE_Ice			SurfaceType12


/*
 * [Named] particle-sound pair...
 */
USTRUCT(BlueprintType, Blueprintable)
struct FMaterialImpactData: public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Visual)
	UParticleSystem* ImpactFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Visual)
    UMaterial* DecalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Sound)
	USoundCue* ImpactSound;

	FMaterialImpactData() : ImpactFX(nullptr), DecalMaterial(nullptr), ImpactSound(nullptr)
	{}

	// TODO: Make it hashable... https://answers.unrealengine.com/questions/707922/tmap-hashable-keytype.html or...
	/*
	 bool operator== (const FMyStruct& Other)
	 {
	   return int32 ID == Other.ID;
	 }
	 friend uint32 GetTypeHash (const FMyStruct& Other)
	 {
	   return GetTypeHash(Other.ID);
	 }
	*/
};


/*
 * Spawnable FX for projectiles' impacts...
 */
UCLASS(Abstract, Blueprintable)
class PROT_API AImpactFX : public AActor
{
	GENERATED_BODY()
	
public:	
	AImpactFX();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	/** spawn effect */
	virtual void PostInitializeComponents() override;

protected:
	/** Spawners... */
	class UDecalComponent* DecalComp;
	class UAudioComponent* AudioComp;

	/** Named impact effects map... */
	UPROPERTY(EditDefaultsOnly, Category=Content)
	TMap<TEnumAsByte<EPhysicalSurface>, FMaterialImpactData> Data;

public:
	/** surface data for spawning */
	UPROPERTY(BlueprintReadOnly, Category=Surface)
	FHitResult SurfaceHit;

protected:
	/** VFX getter... */
	UParticleSystem* GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const;

	/** Decal getter... */
	UMaterial* GetDecal(TEnumAsByte<EPhysicalSurface> SurfaceType) const;

	/** SFX getter... */
	USoundCue* GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const;

};
