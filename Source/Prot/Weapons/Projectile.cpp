// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "Prot.h"
#include "Weapon.h"
#include "ImpactFX.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"


// Sets default values
AProjectile::AProjectile() :
	Speed(300.f)
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->CastShadow = false;
	Mesh->bReceivesDecals = false;
	RootComponent = Mesh;

	// Use a capsule as a simple collision...
	// Make it longer than a projectile to give overlap system possibility to handle it on a high speed...
	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsuleComp"));
	CollisionComp->InitCapsuleSize(2.f, 10.f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	CollisionComp->bTraceComplexOnMove = true;
	CollisionComp->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
	//CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	//CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	//CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");			// Collision profiles are defined in DefaultEngine.ini
	//CollisionComp->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);		// set up a notification for when this component overlaps something
	CollisionComp->SetRelativeTransform(
		FTransform(
			FRotator(90.f, 0.f, 0.f),
			FVector(0.f, -10.f, 0.f),
			FVector(1.f)
		)
	);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = Speed * 100;
	ProjectileMovement->MaxSpeed = 100000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	//ProjectileMovement->ProjectileGravityScale = 1.f;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->SetNetAddressable(); // Make DSO components net addressable
	ProjectileMovement->SetIsReplicated(true); // Enable replication by default

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	SetReplicateMovement(true);

	InitialLifeSpan = 3.f;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (TrailFX)
	{
		TrailPSC = UGameplayStatics::SpawnEmitterAttached(TrailFX, Mesh, FName(""));
	}
}

void AProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &AProjectile::OnImpact);
	//CollisionComp->MoveIgnoreActors.Add(Instigator);

	/*
	AWeapon* OwnerWeapon = Cast<AWeapon>(GetOwner());
	if (OwnerWeapon)
	{
		OwnerWeapon->ApplyWeaponConfig(WeaponConfig);
	}
	*/

	MyController = GetInstigatorController();
}

void AProjectile::OnImpact(const FHitResult& HitResult)
{
	AActor* DamagedActor = HitResult.GetActor();
	if (DamagedActor)
	{
		FString ActorName = DamagedActor->GetName();
		UE_LOG(LogTemp, Warning, TEXT("OnImpact :: %s"), *ActorName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnImpact :: NON"));
	}

	if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		TriggerImpact(HitResult);
		DisableAndDestroy();
	}
}

void AProjectile::TriggerImpact(const FHitResult& Impact)
{
	/* effects and damage origin shouldn't be placed inside mesh at impact point */
	const FVector NudgedImpactLocation = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;
	if (ImpactTemplate)
	{
		FTransform const SpawnTransform(Impact.ImpactNormal.Rotation(), NudgedImpactLocation);
		AImpactFX* const EffectActor = GetWorld()->SpawnActorDeferred<AImpactFX>(ImpactTemplate, SpawnTransform);
		if (EffectActor)
		{
			EffectActor->SurfaceHit = Impact;
			UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
		}
	}
	
	// Damage hit actor if possible...
	AActor* DamagedActor = Impact.GetActor();
	if (DamagedActor)
	{
		FDamageEvent DamageEvent;
		//UGameplayStatics::ApplyDamage(DamagedActor, 3.f, MyController.Get(), this, nullptr);

		// FIXME: Calculate proper damage!
		float DamageToDeal = 2.f;
		DamagedActor->TakeDamage(DamageToDeal, DamageEvent, MyController.Get(), this);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Impact :: No damage is dealt..."));
	}
	bExploded = true;
}

void AProjectile::DisableAndDestroy()
{
	// TODO: Change to ImpactTemplate...
	UAudioComponent* ProjAudioComp = FindComponentByClass<UAudioComponent>();
	if (ProjAudioComp && ProjAudioComp->IsPlaying())
	{
		ProjAudioComp->FadeOut(0.1f, 0.f);
	}

	ProjectileMovement->StopMovementImmediately();

	// give clients some time to show explosion
	SetLifeSpan(2.0f);
}

void AProjectile::OnRep_Exploded()
{
	FVector ProjDirection = GetActorForwardVector();

	const FVector StartTrace = GetActorLocation() - ProjDirection * 200;
	const FVector EndTrace = GetActorLocation() + ProjDirection * 150;
	FHitResult Impact;

	if (!GetWorld()->LineTraceSingleByChannel(Impact, StartTrace, EndTrace, COLLISION_PROJECTILE, FCollisionQueryParams(SCENE_QUERY_STAT(ProjClient), true, GetInstigator())))
	{
		// failsafe
		Impact.ImpactPoint = GetActorLocation();
		Impact.ImpactNormal = -ProjDirection;
	}

	TriggerImpact(Impact);
}

void AProjectile::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = NewVelocity;
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetActorLocation(GetActorLocation() + ProjectileMovement->Velocity * DeltaTime, true);
}

void AProjectile::InitVelocity(const FVector ShootDirection)
{
	if (ProjectileMovement)
	{
		// Velocity Imparted from Owner
		//const FVector ImpartedVel = Instigator ? Instigator->GetVelocity() * InstigatorVelocityPct : FVector::ZeroVector;
		
		ProjectileMovement->Velocity = (ShootDirection * Speed * 100);// +ImpartedVel;
		ProjectileMovement->UpdateComponentVelocity();
	}
}

void AProjectile::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	
}

/*void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor &&
		OtherComp && OtherComp->IsSimulatingPhysics())
	{
		// Spawn emitter...
		// TODO: Make it bloody for NPCs and Characters...
		UWorld* World = GetWorld();
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
		UGameplayStatics::SpawnEmitterAtLocation(World,
			TrailFX,
			OtherActor->GetActorLocation(),
			FRotator::ZeroRotator);

		// Make damage...
		FString ActorName = OtherActor->GetName();
		UE_LOG(LogTemp, Warning, TEXT("OnHit :: %s"), *ActorName);

		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT(ActorName));
		FDamageEvent DamageEvent;
		OtherActor->TakeDamage(3.f, DamageEvent, GetInstigatorController(), this);

		Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnHit :: Nope"));
	}
}*/

void AProjectile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectile, bExploded);
}
