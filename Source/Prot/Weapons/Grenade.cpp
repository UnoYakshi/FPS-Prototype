// Fill out your copyright notice in the Description page of Project Settings.

#include "Grenade.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

// Sets default values
AGrenade::AGrenade()
{
	PrimaryActorTick.bCanEverTick = true;

	SM = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = SM;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	SphereComp->SetupAttachment(RootComponent);

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->bShouldBounce = true;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();

	// Register that function that will be called in any bounce event...
	ProjectileMovementComp->OnProjectileBounce.AddDynamic(this, &AGrenade::OnProjectileBounce);
}

// Called every frame
void AGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGrenade::ArmBomb()
{
	if (bIsArmed)
	{
		//Chance the base color of the static mesh to red
		UMaterialInstanceDynamic* DynamicMAT = SM->CreateAndSetMaterialInstanceDynamic(0);
		DynamicMAT->SetVectorParameterValue(FName("Color"), FLinearColor::Red);
	}
}

void AGrenade::OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	//If the bomb is not armed and we have authority,
	//arm it and perform a delayed explosion
	if (!bIsArmed && Role == ROLE_Authority)
	{
		bIsArmed = true;
		ArmBomb();

		PerformDelayedExplosion(FuseTime);
	}
}

void AGrenade::OnRep_IsArmed()
{
	//Will get called when the bomb is armed 
	//from the authority client
	if (bIsArmed)
	{
		ArmBomb();
	}
}

void AGrenade::PerformDelayedExplosion(float ExplosionDelay)
{
	// Bind Explode() to timer delegate...
	FTimerDelegate TimerDel;
	TimerDel.BindUFunction(this, FName("Explode"));

	// Start countdown...
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, ExplosionDelay, false);
}

void AGrenade::Explode()
{
	SimulateExplosionFX();

	// We won't use any specific damage types in our case
	TSubclassOf<UDamageType> DmgType;
	// Do not ignore any actors
	TArray<AActor*> IgnoreActors;

	//This will eventually call the TakeDamage function that we have overriden in the Character class
	UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), ExplosionRadius, DmgType, IgnoreActors, this, GetInstigatorController());

	// Bind Destroy() the actor to timer delegate...
	FTimerDelegate TimerDel;
	TimerDel.BindLambda([&]()
	{
		Destroy();
	});

	// Destroy the actor after 0.3 seconds... 
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 0.3f, false);
}

void AGrenade::SimulateExplosionFX_Implementation()
{
	if (ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetTransform(), true);
	}
}

void AGrenade::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Tell the engine that we wish to replicate the bIsArmed variable
	DOREPLIFETIME(AGrenade, bIsArmed);
}
