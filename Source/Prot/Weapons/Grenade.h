// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h" 
#include "Components/SphereComponent.h"
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
	UPROPERTY(EditAnywhere)
	UParticleSystem* ExplosionFX;

	/** The static mesh of the comp */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SM;

	/** The projectile movement comp */
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComp;

	/** Sphere comp used for collision. Movement component need a collision component as root to function properly */
	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereComp;

	/** The delay until explosion */
	UPROPERTY(EditDefaultsOnly, Category = BombProps)
	float FuseTime;

	UPROPERTY(EditDefaultsOnly, Category = BombProps)
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = BombProps)
	float ExplosionDamage;

private:
	UPROPERTY(ReplicatedUsing = OnRep_IsArmed)
	bool bIsArmed = false;

/// FUNCTIONALITY
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
