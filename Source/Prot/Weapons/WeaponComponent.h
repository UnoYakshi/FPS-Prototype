// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "Projectile.h"
#include "Magazine.h"

#include "WeaponComponent.generated.h"


class AWeapon;
class UAnimMontage;
class AProtCharacter;
class UAudioComponent;
class UParticleSystemComponent;
class UCameraShake;
class UForceFeedbackEffect;
class USoundCue;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROT_API UWeaponComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:	
	UWeaponComponent();

protected:
	virtual void BeginPlay() override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
	/** Either player pressed Fire action or not (i.e., LMB is pressed)... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	bool bWantsToFire;

	/// //////////////////////////////////////////
	/// FIRE
	UFUNCTION(BlueprintPure, Category = "Game|Weapon")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = "Game|Weapon")
	bool WeaponCanFire() const;

	//////////////////////////////////////////////
	// Handles Character's side start firing...
	//
	/** Player pressed StartFire action (LMB pressed)... */
	UFUNCTION(BlueprintCallable)
	void TryStartFire();

	/** Starts weapon fire... */
	UFUNCTION(BlueprintCallable)
	void JustFire();
	
	//////////////////////////////////////////////
	// Handles Character's side stop firing...
	//
	/** Player pressed StartFire action (LMB released)... */
	UFUNCTION(BlueprintCallable)
	void TryStopFire();

	/** Stops weapon fire... */
	UFUNCTION(BlueprintCallable)
	void JustFireEnd();

	/// //////////////////////////////////////////
	/// RELOAD
	//////////////////////////////////////////////
	// Handles Character's side start reloading...
	//
	/** Player pressed Reload action (R pressed)... */
	UFUNCTION(BlueprintCallable)
	void Reload();

	UFUNCTION(BlueprintCallable)
	bool CanReload();

	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	TSubclassOf<class AMagazine> MagClassToSpawn;

	/// //////////////////////////////////////////
	/// AIM
	//////////////////////////////////////////////
	// Handles camera manipulations for aiming (RMB)...
	//
	UFUNCTION(BlueprintCallable, WithValidation, Server, Reliable, Category = "Weapons")
	virtual void StartAim();
	virtual void StartAim_Implementation();
	bool StartAim_Validate();

	UFUNCTION(BlueprintCallable, WithValidation, Server, Reliable, Category = "Weapons")
	virtual void StopAim();
	virtual void StopAim_Implementation();
	bool StopAim_Validate();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FName WeaponAttachPoint;

protected:
	/** Current weapon using... */
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CurrentWeapon, BlueprintGetter = GetCurrentWeapon)
	AWeapon* CurrentWeapon;

	// INVENTORY
protected:
	/**
	* [server + local] equips weapon from inventory
	*
	* @param Weapon	Weapon to equip
	*/
	void EquipWeapon(class AWeapon* Weapon);

	/** equip weapon */
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerEquipWeapon(class AWeapon* NewWeapon);

	/** CurrentWeapon replication handler... */
	UFUNCTION()
	void OnRep_CurrentWeapon(class AWeapon* LastWeapon);

public:
	/** Updates current weapon...*/
	UFUNCTION(BlueprintCallable)
	void SetCurrentWeapon(class AWeapon* NewWeapon, class AWeapon* LastWeapon = nullptr);


	// UTILITY
public:
	/** Returns Weapon attach point... */
	FName GetCurrentWeaponAttachPoint() const { return WeaponAttachPoint; }

	// Gets current weapon
	UFUNCTION(BlueprintGetter)
	AWeapon* GetCurrentWeapon() { return CurrentWeapon; }
};
