// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MyPlayerController.h"
#include "Weapons/Weapon.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "ProtCharacter.generated.h"

// Either print debug-lines on screen or not...
#define DEBUG false

// Forward declarations...
class AInteractiveObject;


UCLASS(Abstract, config=Game)
class AProtCharacter : public ACharacter
{
	GENERATED_BODY()

	/** For first person (attached to eyes)... */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FPPCamera;

	AProtCharacter();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

public:
	/** Returns Aim Offsets?.. */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	FRotator GetAimOffsets() const;

	/** Weapon's mesh to hold in hands... */
	// TODO: Rework as CurrentWeapon of Weapon class...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	USkeletalMeshComponent* WeaponMesh;

	/** Either player pressed Fire action or not (i.e., LMB is pressed)... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
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
	void JustFire();

	//////////////////////////////////////////////
	// Handles Character's side stop firing...
	//
	/** Player pressed StartFire action (LMB released)... */
	UFUNCTION(BlueprintCallable)
	void TryStopFire();

	/** Stops weapon fire... */
	void JustFireEnd();

	/// //////////////////////////////////////////
	/// RELOAD
	//////////////////////////////////////////////
	// Handles Character's side start reloading...
	//
	/** Player pressed Reload action (R pressed)... */
	void Reload();

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
	/** Current InteractiveObject... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	AInteractiveObject* CurrentInteractive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float MaxUseDistance;

	/** Performs raytrace to find closest looked - at UsableActor.*/
	class AInteractiveObject* GetInteractiveInView();

	/** Basic interaction with InteractiveObjects... */
	UFUNCTION(BlueprintCallable, WithValidation, Server, Reliable, Category = "Interaction")
	virtual void Use();
	virtual void Use_Implementation();
	bool Use_Validate();

	/** Stop the interaction with InteractiveObjects... */
	UFUNCTION(BlueprintCallable, WithValidation, Server, Reliable, Category = "Interaction")
	virtual void StopUsing();
	virtual void StopUsing_Implementation();
	bool StopUsing_Validate();

protected:
	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	/* True first person set-up... */
	AMyPlayerController* PC;

	float CameraTreshold = 20.f;

	float RecoilOffset = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator CameraLocalRotation = FRotator(0.f);

	virtual void PreUpdateCamera(float DeltaTime);
	virtual float CameraProcessYaw(float Input);
	virtual float CameraProcessPitch(float Input);

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

protected:
	FName WeaponAttachPoint;

	/** Currently equipped weapon... */
	UPROPERTY(BlueprintReadWrite, Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	class AWeapon* CurrentWeapon;

protected:
	/** Updates current weapon...*/
	UFUNCTION(BlueprintCallable)
	void SetCurrentWeapon(class AWeapon* NewWeapon, class AWeapon* LastWeapon = nullptr);

	/** CurrentWeapon replication handler... */
	UFUNCTION()
	void OnRep_CurrentWeapon(class AWeapon* LastWeapon);

///////////////////////////////
// Getters...
public:
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFPPCamera() const { return FPPCamera; }

	/** Returns Weapon attach point... */
	FName GetWeaponAttachPoint() const;

};

