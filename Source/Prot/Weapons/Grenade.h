// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h" 
#include "Components/SphereComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Sound/SoundCue.h"
#include "Grenade.generated.h"


UCLASS()
class PROT_API AGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	AGrenade();

	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	/** Marks the properties we wish to replicate */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

/// PARAMETERS
protected:
	UPROPERTY(EditDefaultsOnly)
	USoundCue* ExplosionSFX;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* ExplosionVFX;

	/** Static mesh... */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SM;

	/** The "projectile" movement component; for velocity and bouncing... */
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComp;

	UPROPERTY(VisibleAnywhere)
	URadialForceComponent* RadialForceComp;

	/** 
	 * Sphere component used for collision...
	 * Also, movement component need a collision component as root to function properly...
	 */
	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereComp;

	/** The delay until explosion... */
	UPROPERTY(EditDefaultsOnly, Category = Props)
	float FuseTime;

	/** The explosion's radius... */
	UPROPERTY(EditDefaultsOnly, Category = Props)
	float ExplosionRadius;

	/** The explosion's damage... */
	UPROPERTY(EditDefaultsOnly, Category = Props)
	float ExplosionDamage;

	/** The explosion's strength; in joules?.. */
	UPROPERTY(EditDefaultsOnly, Category = Props)
	float ExplosionStrength;


private:
	/** Either grenade's charged or not... */
	UPROPERTY(ReplicatedUsing = OnRep_IsArmed)
	bool bIsArmed = false;

/// FUNCTIONALITY
public:
	/** Fires the grenade in the given direction; velocity = direction * init_speed... */
	void FireInDirection(const FVector& ShootDirection);

private:
	/** Called when bIsArmed gets updated */
	UFUNCTION()
	void OnRep_IsArmed();

	/** Arms the bomb for explosion */
	void ArmBomb();

	/** Called when our bomb bounces */
	UFUNCTION()
	void OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	/** Performs an explosion after a certain amount of time */
	void PerformDelayedExplosion(float ExplosionDelay);

	/** Performs an explosion when called */
	UFUNCTION()
	void Explode();

	/**
	 * Simulates explosion functions...
	 * The multicast specifier, indicates that every client will call the SimulateExplosionFX_Implementation.
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void SimulateExplosionFX();
	void SimulateExplosionFX_Implementation();
};
