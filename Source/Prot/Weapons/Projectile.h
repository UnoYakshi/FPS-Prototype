// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ParticleDefinitions.h"
#include "Sound/SoundCue.h"
#include "Projectile.generated.h"


UENUM(BlueprintType)
enum class EProjectType : uint8
{
	B762X39 	UMETA(DisplayName = "7.62x39 mm"),
	B556X45 	UMETA(DisplayName = "5.56x45 mm"),
	B223REM		UMETA(DisplayName = ".223 Remington"),
	B45ACP		UMETA(DisplayName = ".45 ACP"),
	B9MM		UMETA(DisplayName = "9 mm"),
	MPORK		UMETA(DisplayName = "Anti-muslim Pork")
};


UCLASS()
class PROT_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

/// PARAMETERS
public:
	/* Type of the bullet... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	EProjectType Type;
	
	/* Projectile's weight... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float Weight;
	
	/* Projectile's low-poly mesh... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
	UStaticMeshComponent* Mesh;

	/* Projectile's trail VFX... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
	UParticleSystemComponent* TrailPSC;

	/* Projectile's on-the-fly SoundCue... */
	USoundCue* FlySC;
};
