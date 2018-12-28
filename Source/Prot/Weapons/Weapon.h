// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Projectile.h"
#include "Magazine.h"
#include "Weapon.generated.h"

class UAnimMontage;
class AProtCharacter;
class UAudioComponent;
class UParticleSystemComponent;
class UCameraShake;
class UForceFeedbackEffect;
class USoundCue;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EWeaponState: uint8
{
	Idle,
	Reloading,
	Firing,
	Equipping
};

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	FString Name;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	FString Description;

	/** time between two consecutive shots */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float TimeBetweenShots;

	/** failsafe reload duration if weapon doesn't have any animation for it */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float NoAnimReloadDuration;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float Damage;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	int AmmoPerShot;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	EProjectileType WeaponProjectileType;

	FWeaponData() :
		Name(FString("")),
		Description(FString("")),
		TimeBetweenShots(0.2f),
		NoAnimReloadDuration(1.f),
		Damage(0.f),
		AmmoPerShot(1),
		WeaponProjectileType(EProjectileType::B545x39)
	{}
};

UCLASS(Abstract, Blueprintable, BlueprintType)
class PROT_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();

private:
	virtual void Tick(float DeltaTime);

	/** perform initial setup */
	virtual void PostInitializeComponents() override;

	virtual void Destroyed() override;

	//////////////////////////////////////////////////////////////////////////
	// Ammo

	/** [server] add ammo */
	void GiveAmmo(int AddAmount);

	/** consume a bullet */
	void UseAmmo();

	/** Returns current ammo type... */
	virtual EProjectileType GetAmmoType() const
	{
		return CurrentMag->Data.ProjectileType;
	}

	//////////////////////////////////////////////////////////////////////////
	// Inventory
public:
	/** weapon is being equipped by owner pawn */
	virtual void OnEquip(const AWeapon* LastWeapon);

	/** weapon is now equipped by owner pawn */
	virtual void OnEquipFinished();

	/** weapon is holstered by owner pawn */
	virtual void OnUnEquip();

	/** [server] weapon was added to pawn's inventory */
	virtual void OnEnterInventory(AProtCharacter* NewOwner);

	/** [server] weapon was removed from pawn's inventory */
	virtual void OnLeaveInventory();

	/** check if it's currently equipped */
	bool IsEquipped() const;

	/** check if mesh is already attached */
	bool IsAttachedToPawn() const;


	//////////////////////////////////////////////////////////////////////////
	// Input
public:
	/** [local + server] start weapon fire */
	virtual void StartFire();

	/** [local + server] stop weapon fire */
	virtual void StopFire();

	/** [all] start weapon reload */
	virtual void StartReload(bool bFromReplication = false);

	/** [local + server] interrupt weapon reload */
	virtual void StopReload();

	/** [server] performs actual reload */
	virtual void ReloadWeapon();

	/** trigger reload from server */
	UFUNCTION(Reliable, Client)
	void ClientStartReload();

	/** Removes CurrentMag... */
	// TODO: Detach magazine's mesh, disown from weapon...
	void RemoveMagazine();

	/** Changes CurrentMag to a new one...
	*   Also checks either EProjectileType is compatible...	
	*/
	void ChangeMagazine(AMagazine* NewMagazine);

	//////////////////////////////////////////////////////////////////////////
	// Control
public:
	/** check if weapon can fire */
	UFUNCTION(BlueprintPure, Category = "Game|Weapon")
	bool CanFire() const;

	/** Check if weapon has magazine and ammo... */
	UFUNCTION(BlueprintPure, Category = "Game|Weapon")
	bool HasAmmo() const;

	/** check if weapon can be reloaded */
	UFUNCTION(BlueprintPure, Category = "Game|Weapon")
	bool CanReload() const;


	//////////////////////////////////////////////////////////////////////////
	// Reading data
public:
	/** get current weapon state */
	EWeaponState GetCurrentState() const;

	/** get weapon mesh (needs pawn owner to determine variant) */
	USkeletalMeshComponent* GetWeaponMesh() const;

	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	class AProtCharacter* GetPawnOwner() const;

	/** set the weapon's owning pawn */
	void SetOwningPawn(AProtCharacter* AShooterCharacter);

	/** gets last time when this weapon was switched to */
	float GetEquipStartedTime() const;

	/** gets the duration of equipping weapon*/
	float GetEquipDuration() const;

protected:
	/** Weapon's pawn owner... */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_MyPawn)
	class AProtCharacter* MyPawn;

	/** Weapon's data... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config)
	FWeaponData WeaponConfig;

protected://TODO: private
	/** Weapon's mesh... VisibleDefaultsOnly*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh;

protected:
	/** Projectile class to spawn... */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = Magazine)
	TSubclassOf<class AMagazine> MagazineClass;

	UPROPERTY(Transient)
	AMagazine* CurrentMag;

	/** firing audio (bLoopedFireSound set) */
	UPROPERTY(Transient)
	UAudioComponent* FireAC;

	/** name of bone/socket for muzzle in weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName MuzzleAttachPoint;

	/** FX for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* MuzzleFX;

	/** spawned component for muzzle FX */
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	/** camera shake on firing */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<UCameraShake> FireCameraShake;

	/** force feedback effect to play when the weapon is fired */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UForceFeedbackEffect *FireForceFeedback;

	/** single fire sound (bLoopedFireSound not set) */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireSound;

	/** looped fire sound (bLoopedFireSound set) */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireLoopSound;

	/** finished burst sound (bLoopedFireSound set) */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireFinishSound;

	/** out of ammo sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* OutOfAmmoSound;

	/** reload sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* ReloadSound;

	/** reload animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* ReloadAnim;

	/** equip sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* EquipSound;

	/** equip animations */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation)
	UAnimMontage* EquipAnim;

	/** fire animations */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation)
	UAnimMontage* FireAnim;

	/** is muzzle FX looped? */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	bool bLoopedMuzzleFX;

	/** is fire sound looped? */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	bool bLoopedFireSound;

	/** is fire animation looped? */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	bool bLoopedFireAnim;

	/** is fire animation playing? */
	UPROPERTY(BlueprintReadOnly)
	bool bPlayingFireAnim;

	/** is weapon currently equipped? */
	bool bIsEquipped;

	/** is weapon fire active? */
	bool bWantsToFire;

	/** is reload animation playing? */
	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing = OnRep_Reload)
	bool bPendingReload;

	/** is equip animation playing? */
	bool bPendingEquip;

	/** weapon is refiring */
	bool bRefiring;

	/** current weapon state */
	EWeaponState CurrentState;

	/** time of last successful weapon fire */
	float LastFireTime;

	/** last time when this weapon was switched to */
	float EquipStartedTime;

	/** how much time weapon needs to be equipped */
	float EquipDuration;

	/** burst counter, used for replicating fire events to remote clients */
	UPROPERTY(VisibleAnywhere, Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

	/** Handle for efficient management of OnEquipFinished timer */
	FTimerHandle TimerHandle_OnEquipFinished;

	/** Handle for efficient management of StopReload timer */
	FTimerHandle TimerHandle_StopReload;

	/** Handle for efficient management of ReloadWeapon timer */
	FTimerHandle TimerHandle_ReloadWeapon;

	/** Handle for efficient management of HandleFiring timer */
	FTimerHandle TimerHandle_HandleFiring;


	//////////////////////////////////////////////////////////////////////////
	// Input - server side

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartFire();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopFire();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartReload();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopReload();


	//////////////////////////////////////////////////////////////////////////
	// Replication & effects

	UFUNCTION()
	void OnRep_MyPawn();

	UFUNCTION()
	void OnRep_BurstCounter();

	UFUNCTION()
	void OnRep_Reload();

	/** Called in network play to do the cosmetic fx for firing */
	virtual void SimulateWeaponFire();

	/** Called in network play to stop cosmetic fx (e.g. for a looping shot). */
	virtual void StopSimulatingWeaponFire();


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon();

	/** [server] fire & update ammo */
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerHandleFiring();
	void ServerHandleFiring_Implementation();
	bool ServerHandleFiring_Validate();

	/** [local + server] handle weapon fire */
	void HandleFiring();

	/** [local + server] firing started */
	virtual void OnBurstStarted();

	/** [local + server] firing finished */
	virtual void OnBurstFinished();

	/** update weapon state */
	void SetWeaponState(EWeaponState NewState);

	/** determine current weapon state */
	void DetermineWeaponState();


	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage helpers

	/** play weapon sounds */
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	/** play weapon animations */
	UFUNCTION(BlueprintCallable)
	float PlayWeaponAnimation(UAnimMontage* Animation);

	/** stop playing weapon animations */
	UFUNCTION(BlueprintCallable)
	void StopWeaponAnimation(UAnimMontage* Animation);

	/** Get the aim of the weapon, allowing for adjustments to be made by the weapon */
	virtual FVector GetAdjustedAim() const;

	/** Get the aim of the camera */
	FVector GetCameraAim() const;

	/** get the originating location for camera damage */
	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	/** get the muzzle location of the weapon */
	FVector GetMuzzleLocation() const;

	/** get direction of weapon's muzzle */
	FVector GetMuzzleDirection() const;

	/** find hit */
	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;

protected:
	/** Returns Mesh subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh; }
	/** Returns Mesh subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh3P() const { return Mesh; }
};

