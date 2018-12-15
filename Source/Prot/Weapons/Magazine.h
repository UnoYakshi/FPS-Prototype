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
	EBulletType BulletType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 MaxCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 CurrentAmmoNum;

	FMagazineData() :
		BulletType(EBulletType::MPORK),
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
	FMagazineData Data;

/// METHODS
public:
	/** Adds ammo to the magazine... */
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	virtual void ConsumeAmmo(int32 NumToConsume);

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	virtual void AddAmmo(int32 NumToAdd);
};
