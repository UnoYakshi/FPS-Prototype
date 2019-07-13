// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
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

#define SURFACE_Snow		SurfaceType9
#define SURFACE_Monster		SurfaceType10
#define SURFACE_Ice			SurfaceType11


/*
 * [Named] particle-sound pair...
 */
USTRUCT()
struct PROT_API FMaterialImpactData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category=FX)
	UParticleSystem* ImpactFX;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* ImpactSound;

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

protected:
	virtual void BeginPlay() override;
	
	/** spawn effect */
	virtual void PostInitializeComponents() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	//static const TMap<FString, int32> TestVar;

	/** Named impact particles and sounds map... */
	UPROPERTY(EditDefaultsOnly, Category=Content)
	TMap<TEnumAsByte<EPhysicalSurface>, FMaterialImpactData> Data;

	///** Default decal when material specific override doesn't exist */
	//UPROPERTY(EditDefaultsOnly, Category=Defaults)
	//struct FDecalData DefaultDecal;

	/** surface data for spawning */
	UPROPERTY(BlueprintReadOnly, Category=Surface)
	FHitResult SurfaceHit;

protected:
	/** get FX for material type */
	UParticleSystem* GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const;

	/** get sound for material type */
	USoundCue* GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const;
};
