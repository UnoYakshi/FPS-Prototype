// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MyPlayerController.h"
#include "Weapons/Weapon.h"
#include "Weapons/WeaponComponent.h"
#include "Weapons/Grenade.h"
#include "HealthActorComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "ProtCharacter.generated.h"

// Forward declarations...
class AInteractiveObject;


UCLASS(Abstract, config=Game)
class AProtCharacter : public ACharacter
{
	GENERATED_BODY()

	/** For first person (attached to eyes)... */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FPPCamera;

public:
	AProtCharacter();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	UHealthActorComponent* HealthComponent;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category=Camera)
	float BaseLookUpRate;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category=Camera)
	float CurTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category=Camera)
	float CurLookUpRate;

	/** Returns Aim Offsets?.. */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	FRotator GetAimOffsets() const;

	// GRENADE & HP
	////////////////////////////////
public:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_GrenadeCount, Category = Stats)
	int32 GrenadeCount;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AGrenade> GrenadeClass;

private:
	/**
	 * TakeDamage Server version. Call this instead of TakeDamage when you're a client
	 * You don't have to generate an implementation. It will automatically call the ServerTakeDamage_Implementation function
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	void ServerTakeDamage_Implementation(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	bool ServerTakeDamage_Validate(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	
	void AttempToSpawnGrenade();

	/** Returns true if we can throw a bomb */
	bool HasGrenades() const { return true; }

	/**
	 * Spawns a bomb. Call this function when you're authorized to.
	 * In case you're not authorized, use the ServerSpawnBomb function.
	 */
	void SpawnGrenade();

	/**
	 * SpawnGrenade Server version. Call this instead of SpawnGrenade when you're a client
	 * You don't have to generate an implementation for this. It will automatically call the ServerSpawnGrenade_Implementation function
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSpawnGrenade();
	void ServerSpawnGrenade_Implementation();
	bool ServerSpawnGrenade_Validate();

	UFUNCTION()
	void OnRep_GrenadeCount();


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

	UFUNCTION(Server, WithValidation, Reliable)
	void TurnAtRateServer(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	UFUNCTION(Server, WithValidation, Reliable)
	void LookUpAtRateServer(float Rate);

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

	float CameraTreshold;

	float RecoilOffset;


	virtual void PreUpdateCamera(float DeltaTime);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FRotator CameraLocalRotation = FRotator(0.f);

	virtual float CameraProcessYaw(float Input);
	virtual float CameraProcessPitch(float Input);

public:
	/** Currently equipped weapon[component]... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Transient)
	UWeaponComponent* CurrentWeaponComp;

protected:
	/** CurrentWeapon replication handler... */
	UFUNCTION()
	void OnRep_CurrentWeapon(class AWeapon* LastWeapon);

///////////////////////////////
// Getters...
public:
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFPPCamera() const { return FPPCamera; }

	// Gets current weapon
	UFUNCTION(BlueprintGetter)
	AWeapon* GetCurrentWeapon() { return CurrentWeaponComp ? CurrentWeaponComp->GetCurrentWeapon() : nullptr; }
};
