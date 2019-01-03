// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"


// Sets default values
AProjectile::AProjectile() :
	Speed(300.f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->CastShadow = false;
	Mesh->bReceivesDecals = false;
	RootComponent = Mesh;

	// Use a capsule as a simple collision...
	// Make it longer than a projectile to give overlap system possibility to handle it on a high speed...
	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsuleComp"));
	CollisionComp->InitCapsuleSize(2.f, 10.f);
	CollisionComp->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");			// Collision profiles are defined in DefaultEngine.ini
	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);		// set up a notification for when this component overlaps something
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
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->SetNetAddressable(); // Make DSO components net addressable
	ProjectileMovement->SetIsReplicated(true); // Enable replication by default

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

// Called every frame
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

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && 
		OtherComp && 
		OtherComp->IsSimulatingPhysics())
	{
		UWorld* World = GetWorld();
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
		UGameplayStatics::SpawnEmitterAtLocation(World, 
			TrailFX, 
			OtherActor->GetActorLocation(), 
			FRotator::ZeroRotator);

		Destroy();
	}
}
