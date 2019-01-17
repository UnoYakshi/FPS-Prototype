// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"

#include "Prot.h"
#include "ProtCharacter.h"
#include "Projectile.h"
#include "MyPlayerController.h"

#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"


AWeapon::AWeapon()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh1P"));
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh->bReceivesDecals = true;
	Mesh->CastShadow = true;
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	RootComponent = Mesh;

	bLoopedMuzzleFX = false;
	bLoopedFireAnim = false;
	bPlayingFireAnim = false;
	bIsEquipped = false;
	bWantsToFire = false;
	bPendingReload = false;
	bPendingEquip = false;
	CurrentState = EWeaponState::Idle;

	BurstCounter = 0;
	LastFireTime = 0.0f;
	CurrentMag = nullptr;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	SetReplicates(true);
	//bNetUseOwnerRelevancy = true;
	//NetUpdateFrequency = 66.0f;
	//MinNetUpdateFrequency = 33.0f;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector Origin = GetMuzzleLocation();
	// Should be GetForwardVector()...	
	// But we have the weapon rotated by 90 degrees counter-clockwise...
	const FVector Direciton = Mesh->GetRightVector();
	const float ProjectileAdjustRange = 10000.0f;

	const FVector EndTrace = Origin + Direciton * ProjectileAdjustRange;

	DrawDebugLine(
		GetWorld(),
		Origin, EndTrace,
		FColor(255, 0, 0),
		false, -1, 0, 3
	);
}

void AWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// TODO: Handle initial ammo in the CurrentMagazine...

	DetachMeshFromPawn();
}

void AWeapon::Destroyed()
{
	Super::Destroyed();

	StopSimulatingWeaponFire();
}

//////////////////////////////////////////////////////////////////////////
// Inventory

void AWeapon::OnEquip(const AWeapon* LastWeapon)
{
	AttachMeshToPawn();

	bPendingEquip = true;
	DetermineWeaponState();

	// Only play animation if last weapon is valid
	if (LastWeapon)
	{
		float Duration = PlayWeaponAnimation(EquipAnim);
		if (Duration <= 0.0f)
		{
			// failsafe
			Duration = 0.5f;
		}
		EquipStartedTime = GetWorld()->GetTimeSeconds();
		EquipDuration = Duration;

		GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &AWeapon::OnEquipFinished, Duration, false);
	}
	else
	{
		OnEquipFinished();
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}

	//AProtCharacter::NotifyEquipWeapon.Broadcast(MyPawn, this);
}

void AWeapon::OnEquipFinished()
{
	AttachMeshToPawn();

	bIsEquipped = true;
	bPendingEquip = false;

	// Determine the state so that the can reload checks will work
	DetermineWeaponState();

	if (MyPawn)
	{
		// TODO: Some functionality upon weapon handling...
		// Initially it was "realoading if needed"...
	}
}

void AWeapon::OnUnEquip()
{
	DetachMeshFromPawn();
	bIsEquipped = false;
	StopFire();

	if (bPendingReload)
	{
		StopWeaponAnimation(ReloadAnim);
		bPendingReload = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_StopReload);
		GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
	}

	if (bPendingEquip)
	{
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_OnEquipFinished);
	}

	//AProtCharacter::NotifyUnEquipWeapon.Broadcast(MyPawn, this);

	DetermineWeaponState();
}

void AWeapon::OnEnterInventory(AProtCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void AWeapon::OnLeaveInventory()
{
	if (Role == ROLE_Authority)
	{
		SetOwningPawn(nullptr);
	}

	if (IsAttachedToPawn())
	{
		OnUnEquip();
	}
}

void AWeapon::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, 
		// bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetWeaponAttachPoint();
		USkeletalMeshComponent* PawnMesh = MyPawn->GetMesh();
		if (MyPawn->IsLocallyControlled())
		{
			Mesh->SetHiddenInGame(false);
			Mesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
		}
		else
		{
			USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
			UseWeaponMesh->SetHiddenInGame(false);
			UseWeaponMesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
		}
	}
}

void AWeapon::DetachMeshFromPawn()
{
	Mesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh->SetHiddenInGame(false);
}


//////////////////////////////////////////////////////////////////////////
// Input

void AWeapon::StartFire()
{
	if (DEBUG)
	{
		switch (Role)
		{
		case ROLE_SimulatedProxy:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Weapon::StartFire::SimProxy!"));
			break;
		case ROLE_AutonomousProxy:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Weapon::StartFire::Client!"));
			break;
		case ROLE_Authority:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Weapon::StartFire::Server!"));
			break;
		default:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Weapon::StartFire::NANI!"));
			break;
		}
	}
	

	if (Role < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void AWeapon::StopFire()
{
	if (DEBUG)
	{
		switch (Role)
		{
		case ROLE_SimulatedProxy:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Weapon::StopFire::SimProxy!"));
			break;
		case ROLE_AutonomousProxy:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Weapon::StopFire::Client!"));
			break;
		case ROLE_Authority:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Weapon::StopFire::Server!"));
			break;
		default:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Weapon::StopFire::NANI!"));
			break;
		}
	}


	if (Role < ROLE_Authority && MyPawn && MyPawn->IsLocallyControlled())
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

void AWeapon::StartReload(bool bFromReplication)
{
	UE_LOG(LogTemp, Warning, TEXT("WEAP::StartReload"));

	// TODO: Check a new magazine's available...

	if (!bFromReplication && Role < ROLE_Authority)
	{
		ServerStartReload();
	}

	if (bFromReplication || CanReload())
	{
		bPendingReload = true;
		DetermineWeaponState();

		float AnimDuration = PlayWeaponAnimation(ReloadAnim);
		if (AnimDuration <= 0.0f)
		{
			AnimDuration = WeaponConfig.NoAnimReloadDuration;
		}
		GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &AWeapon::StopReload, AnimDuration, false);
		if (Role == ROLE_Authority)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &AWeapon::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
		}

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			PlayWeaponSound(ReloadSound);
		}
		UE_LOG(LogTemp, Warning, TEXT("WEAP::StartReload END"));
	}
}

void AWeapon::StopReload()
{
	UE_LOG(LogTemp, Warning, TEXT("WEAP::StopReload"));

	if (CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopWeaponAnimation(ReloadAnim);
		UE_LOG(LogTemp, Warning, TEXT("WEAP::StopReload END"));
	}
}

void AWeapon::FireWeapon()
{
	if (DEBUG || true)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 5.f, FColor::Red, TEXT("Pew!")
		);
	}

	const float ProjectileAdjustRange = 10000.0f;
	const FVector Origin = GetMuzzleLocation();
	const FVector Direciton = Mesh->GetRightVector();
	const FVector EndTrace = Origin + Direciton * ProjectileAdjustRange;
	//const FHitResult Impact = WeaponTrace(Origin, EndTrace);

	ServerFireProjectile(Origin, Direciton);
}

bool AWeapon::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	return true;
}

void AWeapon::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	/*
	if (ProjectileClass)
	{
		UWorld* World = MyPawn->GetWorld();
		AProjectile* NewProjectile = World->SpawnActor<AProjectile>(
			ProjectileClass,
			Origin, Mesh->GetForwardVector().Rotation()
			);
		if (NewProjectile)
		{
			NewProjectile->InitVelocity(Direciton);
			if (DEBUG)
			{
				UE_LOG(LogTemp, Warning, TEXT("Direction: %s"), *Direciton.ToString());
				UE_LOG(LogTemp, Warning, TEXT("Velocity: %s"), *NewProjectile->GetVelocity().ToString());
			}
		}
	}
	//*/

	if (ProjectileClass)
	{
		FTransform SpawnTM(ShootDir.Rotation(), Origin);
		AProjectile* NewProjectile = Cast<AProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileClass, SpawnTM));
		if (NewProjectile)
		{
			NewProjectile->Instigator = Instigator;
			NewProjectile->SetOwner(this);
			NewProjectile->InitVelocity(ShootDir);

			UGameplayStatics::FinishSpawningActor(NewProjectile, SpawnTM);
		}
	}
}

bool AWeapon::ServerStartFire_Validate()
{
	return true;
}

void AWeapon::ServerStartFire_Implementation()
{
	StartFire();
}

bool AWeapon::ServerStopFire_Validate()
{
	return true;
}

void AWeapon::ServerStopFire_Implementation()
{
	StopFire();
}

bool AWeapon::ServerStartReload_Validate()
{
	return true;
}

void AWeapon::ServerStartReload_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("WEAP::ServerStartReload"));
	StartReload();
}

bool AWeapon::ServerStopReload_Validate()
{
	return true;
}

void AWeapon::ServerStopReload_Implementation()
{
	StopReload();
}

void AWeapon::ClientStartReload_Implementation()
{
	StartReload();
}

//////////////////////////////////////////////////////////////////////////
// Control

bool AWeapon::CanFire() const
{
	bool bCanFire = MyPawn && MyPawn->CanFire();
	bool bStateOKToFire = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return (bCanFire && bStateOKToFire && !bPendingReload);
}

bool AWeapon::HasAmmo() const
{
	return CurrentMag && CurrentMag->Data.CurrentAmmoNum > 0;
}

bool AWeapon::CanReload() const
{
	bool bCanReload = (!MyPawn || MyPawn->CanReload());
	// TODO: Check if MyPawn has magazines...
	bool bGotAmmo = HasInfiniteClip();
	bool bStateOKToReload = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return (bCanReload && bGotAmmo && bStateOKToReload);
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage

void AWeapon::GiveAmmo(int AddAmount)
{

}

void AWeapon::UseAmmo()
{
	CurrentMag->ConsumeAmmo(WeaponConfig.AmmoPerShot);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
		TEXT("Ammo: " + FString::FromInt(CurrentMag->Data.CurrentAmmoNum))
	);
}

void AWeapon::HandleFiring()
{
	if (DEBUG)
	{
		switch (Role)
		{
		case ROLE_SimulatedProxy:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Weapon::HandleFiring::SimProxy!"));
			break;
		case ROLE_AutonomousProxy:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Weapon::HandleFiring::Client!"));
			break;
		case ROLE_Authority:
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Weapon::HandleFiring::Server!"));
			break;
		default:
			break;
		}
	}

	if (HasAmmo() && CanFire())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			FireWeapon();

			UseAmmo();

			// update firing FX on remote clients if function was called on server
			++BurstCounter;
			if (DEBUG)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,
					TEXT("GENERAL: " + FString::FromInt(BurstCounter))
				);
			}
		}
	}
	else if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (GetCurrentAmmo() == 0 && !bRefiring)
		{
			PlayWeaponSound(OutOfAmmoSound);
		}

		// Stop weapon fire FX, but stay in Firing state just yet...
		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		// local client will notify server
		if (Role < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// setup refire timer
		bRefiring = (CurrentState == EWeaponState::Firing && WeaponConfig.TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(
				TimerHandle_HandleFiring, this, &AWeapon::HandleFiring, 
				WeaponConfig.TimeBetweenShots, false
			);
		}
	}

	// TODO: Make noise...

	LastFireTime = GetWorld()->GetTimeSeconds();
}

bool AWeapon::ServerHandleFiring_Validate()
{
	return true;
}

void AWeapon::ServerHandleFiring_Implementation()
{
	const bool bShouldUpdateAmmo = (HasAmmo() && CanFire());

	HandleFiring();

	if (bShouldUpdateAmmo)
	{
		// update ammo
		UseAmmo();

		// update firing FX on remote clients
		++BurstCounter;
		if (DEBUG)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,
				TEXT("SERVER: " + FString::FromInt(BurstCounter))
			);
		}
	}
}

void AWeapon::ReloadWeapon()
{

}

void AWeapon::RemoveMagazine()
{
	if (bPendingReload)
	{
		// TODO: Add animation?..
		CurrentMag = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("WEAP::RemoveOldMag"));
	}
}

void AWeapon::ChangeMagazine(AMagazine* NewMagazine)
{
	if (NewMagazine->Data.ProjectileType != WeaponConfig.WeaponProjectileType)
	{
		return;
	}

	bPendingReload = true;
	if (NewMagazine)
	{
		UE_LOG(LogTemp, Warning, TEXT("WEAP::ChangeMag"));
	}

	RemoveMagazine();
	if (NewMagazine != CurrentMag)
	{
		CurrentMag = NewMagazine;
	}
	StartReload();
}

void AWeapon::SetWeaponState(EWeaponState NewState)
{
	const EWeaponState PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void AWeapon::DetermineWeaponState()
{
	EWeaponState NewState = EWeaponState::Idle;

	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if (!CanReload())
			{
				NewState = CurrentState;
			}
			else
			{
				NewState = EWeaponState::Reloading;
			}
		}
		else if (!bPendingReload && bWantsToFire && CanFire())
		{
			NewState = EWeaponState::Firing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState::Equipping;
	}

	SetWeaponState(NewState);
}

void AWeapon::OnBurstStarted()
{
	const float GameTime = GetWorld()->GetTimeSeconds();

	// start firing, can be delayed to satisfy TimeBetweenShots
	if ((LastFireTime > 0 && WeaponConfig.TimeBetweenShots > 0.0f) ||
		LastFireTime + WeaponConfig.TimeBetweenShots <= GameTime)
	{
		HandleFiring();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			TimerHandle_HandleFiring,
			this,
			&AWeapon::HandleFiring,
			LastFireTime + WeaponConfig.TimeBetweenShots - GameTime,
			false
		);
	}
}

void AWeapon::OnBurstFinished()
{
	// stop firing FX on remote clients
	BurstCounter = 0;

	// stop firing FX locally, unless it's a dedicated server
	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulatingWeaponFire();
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage helpers

UAudioComponent* AWeapon::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = nullptr;
	if (Sound && MyPawn)
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, MyPawn->GetRootComponent());
	}

	return AC;
}

float AWeapon::PlayWeaponAnimation(UAnimMontage* Animation)
{
	float Duration = 0.0f;
	if (MyPawn && Animation)
	{
		Duration = MyPawn->PlayAnimMontage(Animation);

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			TEXT("ANIM: " + FString::SanitizeFloat(Duration))
		);
	}

	return Duration;
}

void AWeapon::StopWeaponAnimation(UAnimMontage* Animation)
{
	if (MyPawn && Animation)
	{
		MyPawn->StopAnimMontage(Animation);
	}
}

FVector AWeapon::GetCameraAim() const
{
	AMyPlayerController* const PlayerController = Instigator ? Cast<AMyPlayerController>(Instigator->Controller) : NULL;
	FVector FinalAim = FVector::ZeroVector;

	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	else if (Instigator)
	{
		FinalAim = Instigator->GetBaseAimRotation().Vector();
	}

	return FinalAim;
}

FVector AWeapon::GetAdjustedAim() const
{
	AMyPlayerController* const PlayerController = Instigator ? Cast<AMyPlayerController>(Instigator->Controller) : nullptr;
	FVector FinalAim = FVector::ZeroVector;
	// If we have a player controller use it for the aim
	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	/*else if (Instigator)
	{
		// Now see if we have an AI controller - we will want to get the aim from there if we do
		AShooterAIController* AIController = MyPawn ? Cast<AShooterAIController>(MyPawn->Controller) : NULL;
		if (AIController != NULL)
		{
			FinalAim = AIController->GetControlRotation().Vector();
		}
		else
		{
			FinalAim = Instigator->GetBaseAimRotation().Vector();
		}
	}*/

	return FinalAim;
}

FVector AWeapon::GetCameraDamageStartLocation(const FVector& AimDir) const
{
	AMyPlayerController* PC = MyPawn ? Cast<AMyPlayerController>(MyPawn->Controller) : NULL;
	//AShooterAIController* AIPC = MyPawn ? Cast<AShooterAIController>(MyPawn->Controller) : NULL;
	FVector OutStartTrace = FVector::ZeroVector;

	if (PC)
	{
		// use player's camera
		FRotator UnusedRot;
		PC->GetPlayerViewPoint(OutStartTrace, UnusedRot);

		// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
		OutStartTrace = OutStartTrace + AimDir * ((Instigator->GetActorLocation() - OutStartTrace) | AimDir);
	}
	/*
	else if (AIPC)
	{
		OutStartTrace = GetMuzzleLocation();
	}
	//*/

	return OutStartTrace;
}

FVector AWeapon::GetMuzzleLocation() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector AWeapon::GetMuzzleDirection() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

FHitResult AWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, Instigator);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams);

	return Hit;
}

void AWeapon::SetOwningPawn(AProtCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		Instigator = NewOwner;
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}
}

//////////////////////////////////////////////////////////////////////////
// Replication & effects

void AWeapon::OnRep_MyPawn()
{
	if (MyPawn)
	{
		OnEnterInventory(MyPawn);
	}
	else
	{
		OnLeaveInventory();
	}
}

void AWeapon::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateWeaponFire();
	}
	else
	{
		StopSimulatingWeaponFire();
	}
}

void AWeapon::OnRep_Reload()
{
	if (bPendingReload)
	{
		StartReload(true);
	}
	else
	{
		StopReload();
	}
}

void AWeapon::SimulateWeaponFire()
{
	if (Role == ROLE_Authority && CurrentState != EWeaponState::Firing)
	{
		return;
	}
	
	if (MuzzleFX)
	{
		MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh, MuzzleAttachPoint);
	}

	if (!bLoopedFireAnim || !bPlayingFireAnim)
	//if (!bPlayingFireAnim)
	{
		PlayWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	if (bLoopedFireSound)
	{
		if (FireAC == NULL)
		{
			FireAC = PlayWeaponSound(FireLoopSound);
		}
	}
	else
	{
		PlayWeaponSound(FireSound);
	}
}

void AWeapon::StopSimulatingWeaponFire()
{
	if (bLoopedMuzzleFX)
	{
		if (MuzzlePSC)
		{
			MuzzlePSC->DeactivateSystem();
			MuzzlePSC = nullptr;
		}
	}

	if (bLoopedFireAnim && bPlayingFireAnim)
	{
		StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}

	if (FireAC)
	{
		FireAC->FadeOut(0.1f, 0.0f);
		FireAC = nullptr;

		PlayWeaponSound(FireFinishSound);
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, MyPawn);

	DOREPLIFETIME_CONDITION(AWeapon, CurrentMag, COND_OwnerOnly);

	DOREPLIFETIME_CONDITION(AWeapon, BurstCounter, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWeapon, bPendingReload, COND_SkipOwner);
}

USkeletalMeshComponent* AWeapon::GetWeaponMesh() const
{
	return Mesh;
}

class AProtCharacter* AWeapon::GetPawnOwner() const
{
	return MyPawn;
}

bool AWeapon::IsEquipped() const
{
	return bIsEquipped;
}

bool AWeapon::IsAttachedToPawn() const
{
	return bIsEquipped || bPendingEquip;
}

EWeaponState AWeapon::GetCurrentState() const
{
	return CurrentState;
}

float AWeapon::GetEquipStartedTime() const
{
	return EquipStartedTime;
}

float AWeapon::GetEquipDuration() const
{
	return EquipDuration;
}
