// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "GameFramework/Actor.h"
#include "Magazine.generated.h"


USTRUCT(BlueprintType)
struct FMagazineData
{
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	EProjectType ProjectileType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 MaxCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 CurrentAmmoNum;

	FMagazineData() :
		ProjectileType(EProjectType::MPORK),
		MaxCapacity(0),
		CurrentAmmoNum(0)
	{}
};


UCLASS()
class PROT_API AMagazine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagazine();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


/// PARAMETERS
public:
	/* Magazine's config... */
	FMagazineData Data;

/// METHODS
public:
	/** Consumes ammo from the magazine [if there is enough ammo, otherwise do nothing]...  */
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	virtual void ConsumeAmmo(int32 NumToConsume);

	/* Adds ammo to the magazine [clamping the end value between 0 and MaxCapacity]... */
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	virtual void AddAmmo(int32 NumToAdd);

};
