// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

#include "Weapons/WeaponComponent.h"
#include "Weapons/FireInterface.h"

#include "Interactive/InteractiveObject.h"
#include "MyPlayerController.h"
#include "HealthActorComponent.h"
#include "Weapons/Grenade.h"

#include "BaseCharacter.generated.h"

// Forward declarations...
class AInteractiveObject;


UCLASS()
class PROT_API ABaseCharacter : public ACharacter, public IFireInterface
{
	GENERATED_BODY()

	/** For first person (attached to eyes)... */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FPPCamera;
public:
	ABaseCharacter();
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	UWeaponComponent* WeaponComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	UHealthActorComponent* HealthComponent;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

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
	////////////////////////////////

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UWeaponComponent* WeaponComp;

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

	float CameraTreshold;

	float RecoilOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FRotator CameraLocalRotation = FRotator(0.f);

	virtual void PreUpdateCamera(float DeltaTime);
	virtual float CameraProcessYaw(float Input);
	virtual float CameraProcessPitch(float Input);


///////////////////////////////
// Getters...
public:
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFPPCamera() const { return FPPCamera; }

};
