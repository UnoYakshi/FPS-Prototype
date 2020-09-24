// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ParticleDefinitions.h"
#include "Sound/SoundCue.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "ImpactFX.h"
#include "Projectile.generated.h"


class AImpactFX;


UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	B762X39 	UMETA(DisplayName = "7.62x39 mm"),
	B545x39		UMETA(DisplayName = "5.45x39 mm"),
	B556X45 	UMETA(DisplayName = "5.56x45 mm"),
	B45ACP		UMETA(DisplayName = ".45 ACP"),
	B9MM		UMETA(DisplayName = "9 mm"),
	MPORK		UMETA(DisplayName = "Anti-muslim Pork")
};


UCLASS(config=Game)
class PROT_API AProjectile : public AActor
{
	GENERATED_BODY()

	/** initial setup */
	virtual void PostInitializeComponents() override;

	/** handle hit */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);

public:	
	AProjectile();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	//UFUNCTION()
	//void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void InitVelocity(const FVector ShootDirection);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

/// PARAMETERS
protected:
	/* Type of the bullet... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	EProjectileType Type;
	
	/* Projectile's weight, in kilos(?)... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float Weight;

	/* Projectile's life span, i.e., how long it will live, in seconds... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float LifeSpan;

	/* Projectile's low-poly mesh... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
	UStaticMeshComponent* Mesh;

	/* Projectile's trail VFX... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
	UParticleSystem* TrailFX;

	UPROPERTY(Transient)
	UParticleSystemComponent* TrailPSC;

	/* Projectile's on-the-fly SoundCue... */
	USoundCue* FlySC;

	/** Sphere collision component */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	UCapsuleComponent* CollisionComp;

	/** controller that fired me (cache for damage calculations) */
	TWeakObjectPtr<AController> MyController;

	/** did it explode? */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Exploded)
	bool bExploded;

	/** Projectile's speed, in m/s... */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float Speed = 735.f;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TSubclassOf<AImpactFX> ImpactTemplate;


protected:
	/** Trigger impact (FXs)... */
	void TriggerImpact(const FHitResult& Impact);

	/** shutdown projectile and prepare for destruction */
	void DisableAndDestroy();

	/// REPLICATION
	/** [client] explosion happened */
	UFUNCTION()
	void OnRep_Exploded();

	/** Update velocity on client... */
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;


protected:
	/** Returns MovementComp subobject **/
	FORCEINLINE UProjectileMovementComponent* GetMovementComp() const { return ProjectileMovement; }
	/** Returns CollisionComp subobject **/
	FORCEINLINE UCapsuleComponent* GetCollisionComp() const { return CollisionComp; }
};
