// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Magazine.h"
#include "Weapon.generated.h"
class AProtCharacter;

UENUM(BlueprintType)
enum class WeaponState : uint8
{
		Idle,
		Firing,
		Reloading,
		Equipping
};

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	float FireRate;//Here we mean Time Between Shots

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	float BulletSpeed;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	float Damage;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	float ReloadTime;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	bool bIsFiring;

	///TODO : Default Values
};

USTRUCT(BlueprintType)
struct FWeaponContent
{
	GENERATED_USTRUCT_BODY()
	
	/// TODO: Make it UParticleSystemComponent instead...
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* ImpactFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerFX;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	USoundCue* FireCue;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	USoundCue* FireLoopedCue;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	USoundCue* FireFinishedCue;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	USoundCue* OutOfAmmoCue;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	USoundCue* ReloadCue;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	USoundCue* EquipCue;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCamShake;

	FWeaponContent() :
		ImpactFX(nullptr),
		TracerFX(nullptr),
		MuzzleFX(nullptr),
		FireCamShake(nullptr)
	{
	}
};

USTRUCT(BlueprintType)
struct FWeaponAnim
{
	GENERATED_USTRUCT_BODY()

	/** animation played on pawn (1st person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Pawn1P;
};

UCLASS(Blueprintable)
class PROT_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	AProtCharacter* Carrier;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	USkeletalMeshComponent* SKMeshComp;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	AMagazine *CurrentMagazine;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FWeaponData WeaponData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FWeaponContent WeaponContent;
	
protected:
	void PlayFireEffects(FVector TraceEnd);
	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	void Fire();

	FTimerHandle TimerHandle_TimeBetweenShots;
	float LastFireTime;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

public:
	void Destroyed();


	void StartFire();
	void StopFire();
	void StartReload();
	void StopReload();
	void UseAmmo();
	virtual void OnEquip(const AWeapon* LastWeapon);
	void OnEquipFinished();
	void ServerStartFire();
	void ServerStopFire();
	void ServerStartReload();
	void ServerStopReload();
	void ServerHandleFiring();
	void ClientHandleFiring();
	void ClientFire();
	bool CanReload();
	bool CanFire();
};
