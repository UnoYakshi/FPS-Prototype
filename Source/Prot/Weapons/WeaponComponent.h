// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "Projectile.h"
#include "Magazine.h"

#include "WeaponComponent.generated.h"


class UAnimMontage;
class AProtCharacter;
class UAudioComponent;
class UParticleSystemComponent;
class UCameraShake;
class UForceFeedbackEffect;
class USoundCue;


UENUM(BlueprintType)  //"BlueprintType" is essential to include
enum class EWeaponState2 : uint8
{
	Idle,
	Reloading,
	Firing,
	Equipping
};


USTRUCT(BlueprintType)
struct FWeaponData2
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

	FWeaponData2() :
		Name(FString("")),
		Description(FString("")),
		TimeBetweenShots(0.2f),
		NoAnimReloadDuration(1.f),
		Damage(0.f),
		AmmoPerShot(1),
		WeaponProjectileType(EProjectileType::B545x39)
	{}
};


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


	//////////////////////////////////////////////////////////////////////////
	// Util
protected:
	/** Weapon's mesh... VisibleDefaultsOnly*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	USkeletalMeshComponent* MeshComp;

	/** Weapon's data... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FWeaponData2 WeaponConfig;

	/** Socket's name for muzzle in the weapon's mesh... */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	FName MuzzleAttachPoint;

	/** current weapon state */
	EWeaponState2 CurrentState;

	/** GetOwner() casted to APawn...*/
	UPROPERTY(Transient, ReplicatedUsing = OnRep_MyPawn)
	APawn* MyPawn;

protected:
	/** update weapon state */
	void SetWeaponState(EWeaponState2 NewState);

	/** determine current weapon state */
	void DetermineWeaponState();

	/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();

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

	UFUNCTION()
	void OnRep_MyPawn();

	/** Returns Mesh subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return MeshComp; }

public:
	/** get current weapon state */
	EWeaponState2 GetCurrentState() const;

	/** get weapon mesh (needs pawn owner to determine variant) */
	USkeletalMeshComponent* GetWeaponMesh() const;


	//////////////////////////////////////////////////////////////////////////
	// Ammo
protected:
	/** Projectile class to spawn... */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = Magazine)
	TSubclassOf<class AMagazine> MagazineClass;

	UPROPERTY(Transient)
	AMagazine* CurrentMag;

	/** out of ammo sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* OutOfAmmoSound;

protected:
	/** [server] Adds ammo... */
	void GiveAmmo(int AddAmount);

	/** Consumes a bullet... */
	void UseAmmo();

public:
	/** Returns current ammo type... */
	virtual EProjectileType GetAmmoType() const
	{
		return CurrentMag->Data.ProjectileType;
	}

	/** Returns the number of projectiles in current magazine (if some)... */
	virtual int32 GetAmmoNum() const
	{
		return CurrentMag ? CurrentMag->GetCurrentAmmo() : 0;
	}

	/** Check if weapon has magazine and ammo... */
	UFUNCTION(BlueprintPure, Category = "Game|Weapon")
	bool HasAmmo() const;


	//////////////////////////////////////////////////////////////////////////
	// Reload
protected:
	/** reload sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* ReloadSound;

	/** reload animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* ReloadAnim;

	/** is reload animation playing? */
	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing = OnRep_Reload)
	bool bPendingReload;

	/** Handle for efficient management of ReloadWeapon timer */
	FTimerHandle TimerHandle_ReloadWeapon;

	/** Handle for efficient management of StopReload timer */
	FTimerHandle TimerHandle_StopReload;

protected:
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartReload();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopReload();

	UFUNCTION()
	void OnRep_Reload();

public:
	/** [all] start weapon reload */
	UFUNCTION(BlueprintCallable)
	virtual void StartReload(bool bFromReplication = false);

	/** [local + server] interrupt weapon reload */
	UFUNCTION(BlueprintCallable)
	virtual void StopReload();

	/** [server] performs actual reload */
	UFUNCTION(BlueprintCallable)
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

public:
	/** Checks if weapon can be reloaded.... */
	UFUNCTION(BlueprintPure, Category = "Game|Weapon")
	bool CanReload() const;


	//////////////////////////////////////////////////////////////////////////
	// Fire
protected:
	/** firing audio (bLoopedFireSound set) */
	UPROPERTY(Transient)
	UAudioComponent* FireAC;

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

	/** is weapon fire active? */
	bool bWantsToFire;

	/** weapon is refiring */
	bool bRefiring;

	/** time of last successful weapon fire */
	float LastFireTime;

	/** burst counter, used for replicating fire events to remote clients */
	UPROPERTY(VisibleAnywhere, Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

	/** Handle for efficient management of HandleFiring timer */
	FTimerHandle TimerHandle_HandleFiring;

protected:
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartFire();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopFire();

	UFUNCTION()
	void OnRep_BurstCounter();

	/** Called in network play to do the cosmetic fx for firing */
	virtual void SimulateWeaponFire();

	/** Called in network play to stop cosmetic fx (e.g. for a looping shot). */
	virtual void StopSimulatingWeaponFire();

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

public:
	/** [local + server] start weapon fire */
	UFUNCTION(BlueprintCallable)
	virtual void StartFire();

	/** [local + server] stop weapon fire */
	UFUNCTION(BlueprintCallable)
	virtual void StopFire();

	/** check if weapon can fire */
	UFUNCTION(BlueprintPure, Category = "Game|Weapon")
	bool CanFire() const;


	//////////////////////////////////////////////////////////////////////////
	// Equip
protected:
	/** equip sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* EquipSound;

	/** equip animations */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animation)
	UAnimMontage* EquipAnim;
	
	/** is weapon currently equipped? */
	bool bIsEquipped;

	/** is equip animation playing? */
	bool bPendingEquip;

	/** last time when this weapon was switched to */
	float EquipStartedTime;

	/** how much time weapon needs to be equipped */
	float EquipDuration;

	/** Handle for efficient management of OnEquipFinished timer */
	FTimerHandle TimerHandle_OnEquipFinished;

public:
	/** weapon is being equipped by owner pawn */
	virtual void OnEquip(const UWeaponComponent* LastWeapon);

	/** weapon is now equipped by owner pawn */
	virtual void OnEquipFinished();

	/** weapon is holstered by owner pawn */
	virtual void OnUnEquip();

	/** [server] weapon was added to pawn's inventory */
	virtual void OnEnterInventory(APawn* NewOwner);

	/** [server] weapon was removed from pawn's inventory */
	virtual void OnLeaveInventory();

	/** check if it's currently equipped */
	bool IsEquipped() const;

	/** check if mesh is already attached */
	bool IsAttachedToPawn() const;

	/** set the weapon's owning pawn */
	void SetOwningPawn(APawn* NewPawn);

	/** gets last time when this weapon was switched to */
	float GetEquipStartedTime() const;

	/** gets the duration of equipping weapon*/
	float GetEquipDuration() const;
};
