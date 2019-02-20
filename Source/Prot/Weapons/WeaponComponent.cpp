// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponComponent.h"

#include "Prot.h"
#include "Projectile.h"
#include "MyPlayerController.h"

#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"


UWeaponComponent::UWeaponComponent()
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

	bLoopedMuzzleFX = false;
	bLoopedFireAnim = false;
	bPlayingFireAnim = false;
	bIsEquipped = false;
	bWantsToFire = false;
	bPendingReload = false;
	bPendingEquip = false;
	CurrentState = EWeaponState2::Idle;

	BurstCounter = 0;
	LastFireTime = 0.0f;

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	//SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	//SetReplicates(true);
	//bNetUseOwnerRelevancy = true;
	//NetUpdateFrequency = 66.0f;
	//MinNetUpdateFrequency = 33.0f;

	MyPawn = Cast<APawn>(GetOwner());
}

void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const FVector Origin = GetMuzzleLocation();
	// Should be GetForwardVector()...	
	// But we have the weapon rotated by 90 degrees counter-clockwise...
	const FVector Direciton = Mesh->GetRightVector();
	const float ProjectileAdjustRange = 10000.0f;

	const FVector EndTrace = Origin + Direciton * ProjectileAdjustRange;

	if (DEBUG || true)  // TODO: Remove `|| true`...
	{
		DrawDebugLine(
			GetWorld(),
			Origin, EndTrace,
			FColor(255, 0, 0),
			false,
			-1,
			0,
			3
		);
	}
}

//void UWeaponComponent::PostInitProperties()
//{
//	Super::PostInitProperties();
//
//	// TODO: Handle initial ammo in the CurrentMagazine...
//
//	DetachMeshFromPawn();
//}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	DetachMeshFromPawn();
}

void UWeaponComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	StopSimulatingWeaponFire();
}

//////////////////////////////////////////////////////////////////////////
// Inventory

void UWeaponComponent::OnEquip(const UWeaponComponent* LastWeapon)
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

		GetWorld()->GetTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &UWeaponComponent::OnEquipFinished, Duration, false);
	}
	else
	{
		OnEquipFinished();
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}
}

void UWeaponComponent::OnEquipFinished()
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

void UWeaponComponent::OnUnEquip()
{
	DetachMeshFromPawn();
	bIsEquipped = false;
	StopFire();

	if (bPendingReload)
	{
		StopWeaponAnimation(ReloadAnim);
		bPendingReload = false;

		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_StopReload);
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
	}

	if (bPendingEquip)
	{
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_OnEquipFinished);
	}

	DetermineWeaponState();
}

void UWeaponComponent::OnEnterInventory(APawn* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void UWeaponComponent::OnLeaveInventory()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		SetOwningPawn(nullptr);
	}

	if (IsAttachedToPawn())
	{
		OnUnEquip();
	}
}

void UWeaponComponent::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, 
		// bOwnerNoSee flags deal with visibility.
		//FName AttachPoint = MyPawn->GetWeaponAttachPoint();
		//USkeletalMeshComponent* PawnMesh = MyPawn->GetMesh();
		FName AttachPoint = FName("");
		USkeletalMeshComponent* PawnMesh = nullptr;
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

void UWeaponComponent::DetachMeshFromPawn()
{
	Mesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh->SetHiddenInGame(false);
}


//////////////////////////////////////////////////////////////////////////
// Input

void UWeaponComponent::StartFire()
{
	if (DEBUG)
	{
		switch (GetOwnerRole())
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


	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void UWeaponComponent::StopFire()
{
	if (DEBUG)
	{
		switch (GetOwnerRole())
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


	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

void UWeaponComponent::StartReload(bool bFromReplication)
{
	if (DEBUG)
	{
		UE_LOG(LogTemp, Warning, TEXT("WEAP::StartReload"));
	}

	if (!bFromReplication && GetOwnerRole() < ROLE_Authority)
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
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_StopReload, this, &UWeaponComponent::StopReload, AnimDuration, false);
		if (GetOwnerRole() == ROLE_Authority)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &UWeaponComponent::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
		}

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			PlayWeaponSound(ReloadSound);
		}
		if (DEBUG)
		{
			UE_LOG(LogTemp, Warning, TEXT("WEAP::StartReload END"));
		}
	}
}

void UWeaponComponent::StopReload()
{
	if (DEBUG)
	{
		UE_LOG(LogTemp, Warning, TEXT("WEAP::StopReload"));
	}

	if (CurrentMag)
	{

	}
	if (CurrentState == EWeaponState2::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopWeaponAnimation(ReloadAnim);
		if (DEBUG)
		{
			UE_LOG(LogTemp, Warning, TEXT("WEAP::StopReload END"));
		}
	}
}

void UWeaponComponent::FireWeapon()
{
	if (DEBUG)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 5.f, FColor::Red, TEXT("Pew!")
		);
	}

	const FVector Origin = GetMuzzleLocation();
	const FVector Direciton = Mesh->GetRightVector();
	const float ProjectileAdjustRange = 10000.0f;
	const FVector EndTrace = Origin + Direciton * ProjectileAdjustRange;

	if (ProjectileClass)
	{
		UWorld* World = GetWorld();
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
}

bool UWeaponComponent::ServerStartFire_Validate()
{
	return true;
}

void UWeaponComponent::ServerStartFire_Implementation()
{
	StartFire();
}

bool UWeaponComponent::ServerStopFire_Validate()
{
	return true;
}

void UWeaponComponent::ServerStopFire_Implementation()
{
	StopFire();
}

bool UWeaponComponent::ServerStartReload_Validate()
{
	return true;
}

void UWeaponComponent::ServerStartReload_Implementation()
{
	if (DEBUG)
	{
		UE_LOG(LogTemp, Warning, TEXT("WEAP::ServerStartReload"));
	}
	StartReload();
}

bool UWeaponComponent::ServerStopReload_Validate()
{
	return true;
}

void UWeaponComponent::ServerStopReload_Implementation()
{
	StopReload();
}

void UWeaponComponent::ClientStartReload_Implementation()
{
	StartReload();
}

//////////////////////////////////////////////////////////////////////////
// Control

bool UWeaponComponent::CanFire() const
{
	/*if (MyPawn->GetClass()->ImplementsInterface(UAnimMontageInterface::StaticClass()))
	{
		bool bCanFire = MyPawn && UAnimMontageInterface::Execute_CanFire();
	}*/
	bool bCanFire = true;
	bool bStateOKToFire = ((CurrentState == EWeaponState2::Idle) || (CurrentState == EWeaponState2::Firing));
	return (bCanFire && bStateOKToFire && !bPendingReload);
}

bool UWeaponComponent::HasAmmo() const
{
	return CurrentMag && CurrentMag->Data.CurrentAmmoNum > 0;
}

bool UWeaponComponent::CanReload() const
{
	//TODO: bool bCanReload = (!MyPawn || MyPawn->CanReload());
	// TODO: Check if MyPawn has magazines...
	bool bCanReload = (MyPawn);
	bool bGotAmmo = true;

	bool bStateOKToReload = ((CurrentState == EWeaponState2::Idle) || (CurrentState == EWeaponState2::Firing));
	return (bCanReload && bGotAmmo && bStateOKToReload);
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage

void UWeaponComponent::GiveAmmo(int AddAmount)
{

}

void UWeaponComponent::UseAmmo()
{
	CurrentMag->ConsumeAmmo(WeaponConfig.AmmoPerShot);
	if (DEBUG)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			TEXT("Ammo: " + FString::FromInt(CurrentMag->Data.CurrentAmmoNum))
		);
	}
}

void UWeaponComponent::HandleFiring()
{
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
		if (!bRefiring)
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
		if (GetOwnerRole() < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// setup refire timer
		bRefiring = (CurrentState == EWeaponState2::Firing && WeaponConfig.TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle_HandleFiring, this, &UWeaponComponent::HandleFiring,
				WeaponConfig.TimeBetweenShots, false
			);
		}
	}

	// TODO: Make noise...

	LastFireTime = GetWorld()->GetTimeSeconds();
}

bool UWeaponComponent::ServerHandleFiring_Validate()
{
	return true;
}

void UWeaponComponent::ServerHandleFiring_Implementation()
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

void UWeaponComponent::ReloadWeapon()
{

}

void UWeaponComponent::RemoveMagazine()
{
	if (bPendingReload)
	{
		// TODO: Add animation?..
		CurrentMag = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("WEAP::RemoveOldMag"));
	}
}

void UWeaponComponent::ChangeMagazine(AMagazine* NewMagazine)
{
	if (NewMagazine->Data.ProjectileType != WeaponConfig.WeaponProjectileType)
	{
		return;
	}

	bPendingReload = true;
	if (NewMagazine)
	{
		if (DEBUG)
		{
			UE_LOG(LogTemp, Warning, TEXT("WEAP::ChangeMag"));
		}
	}

	RemoveMagazine();
	if (NewMagazine != CurrentMag)
	{
		CurrentMag = NewMagazine;
	}
	StartReload();
}

void UWeaponComponent::SetWeaponState(EWeaponState2 NewState)
{
	const EWeaponState2 PrevState = CurrentState;

	if (PrevState == EWeaponState2::Firing && NewState != EWeaponState2::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState2::Firing && NewState == EWeaponState2::Firing)
	{
		OnBurstStarted();
	}
}

void UWeaponComponent::DetermineWeaponState()
{
	EWeaponState2 NewState = EWeaponState2::Idle;

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
				NewState = EWeaponState2::Reloading;
			}
		}
		else if (!bPendingReload && bWantsToFire && CanFire())
		{
			NewState = EWeaponState2::Firing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState2::Equipping;
	}

	SetWeaponState(NewState);
}

void UWeaponComponent::OnBurstStarted()
{
	// start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 
		&& WeaponConfig.TimeBetweenShots > 0.0f
		&& LastFireTime + WeaponConfig.TimeBetweenShots > GameTime)
	{
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_HandleFiring,
			this,
			&UWeaponComponent::HandleFiring,
			LastFireTime + WeaponConfig.TimeBetweenShots - GameTime,
			false
		);
	}
	else
	{
		HandleFiring();
	}
}

void UWeaponComponent::OnBurstFinished()
{
	// stop firing FX on remote clients
	BurstCounter = 0;

	// stop firing FX locally, unless it's a dedicated server
	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulatingWeaponFire();
	}

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage helpers

UAudioComponent* UWeaponComponent::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = nullptr;
	if (Sound && MyPawn)
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, MyPawn->GetRootComponent());
	}

	return AC;
}

float UWeaponComponent::PlayWeaponAnimation(UAnimMontage* Animation)
{
	float Duration = 0.0f;
	if (MyPawn && Animation)
	{
		//Duration = MyPawn->PlayAnimMontage(Animation);
		//if (MyPawn->GetClass()->ImplementsInterface(UAnimMontageInterface::StaticClass()))
		//{
		//	UAnimMontageInterface::Execute_PlayAnimMontage(Animation);
		//}
		if (DEBUG)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
				TEXT("ANIM: " + FString::SanitizeFloat(Duration))
			);
		}
	}

	return Duration;
}

void UWeaponComponent::StopWeaponAnimation(UAnimMontage* Animation)
{
	//if (MyPawn && Animation)
	//{
	//	// MyPawn->StopAnimMontage(Animation);
	//}
	//if (MyPawn->GetClass()->ImplementsInterface(UAnimMontageInterface::StaticClass()))
	//{
	//	UAnimMontageInterface::Execute_StopAnimMontage(Animation);
	//}
}

FVector UWeaponComponent::GetCameraAim() const
{
	AMyPlayerController* const PlayerController = MyPawn->Instigator ? Cast<AMyPlayerController>(MyPawn->GetInstigatorController()) : nullptr;
	FVector FinalAim = FVector::ZeroVector;

	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	else if (MyPawn->Instigator)
	{
		FinalAim = MyPawn->Instigator->GetBaseAimRotation().Vector();
	}

	return FinalAim;
}

FVector UWeaponComponent::GetAdjustedAim() const
{
	AMyPlayerController* const PlayerController = MyPawn->Instigator ? Cast<AMyPlayerController>(MyPawn->Instigator->Controller) : nullptr;
	FVector FinalAim = FVector::ZeroVector;
	// If we have a player controller use it for the aim
	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	/*else if (MyPawn->Instigator)
	{
		// Now see if we have an AI controller - we will want to get the aim from there if we do
		AShooterAIController* AIController = MyPawn ? Cast<AShooterAIController>(MyPawn->Controller) : NULL;
		if (AIController != NULL)
		{
			FinalAim = AIController->GetControlRotation().Vector();
		}
		else
		{
			FinalAim = MyPawn->Instigator->GetBaseAimRotation().Vector();
		}
	}*/

	return FinalAim;
}

FVector UWeaponComponent::GetCameraDamageStartLocation(const FVector& AimDir) const
{
	AMyPlayerController* PC = MyPawn ? Cast<AMyPlayerController>(MyPawn->Controller) : nullptr;
	//ABotAIC* AIPC = MyPawn ? Cast<ABotAIC>(MyPawn->Controller) : nullptr;
	FVector OutStartTrace = FVector::ZeroVector;

	if (PC)
	{
		// use player's camera
		FRotator UnusedRot;
		PC->GetPlayerViewPoint(OutStartTrace, UnusedRot);

		// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
		OutStartTrace = OutStartTrace + AimDir * ((MyPawn->Instigator->GetActorLocation() - OutStartTrace) | AimDir);
	}
	/*
	else if (AIPC)
	{
		OutStartTrace = GetMuzzleLocation();
	}
	//*/

	return OutStartTrace;
}

FVector UWeaponComponent::GetMuzzleLocation() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector UWeaponComponent::GetMuzzleDirection() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

FHitResult UWeaponComponent::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, MyPawn->Instigator);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams);

	return Hit;
}

void UWeaponComponent::SetOwningPawn(APawn* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		MyPawn = NewOwner;
	}
}

//////////////////////////////////////////////////////////////////////////
// Replication & effects

void UWeaponComponent::OnRep_MyPawn()
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

void UWeaponComponent::OnRep_BurstCounter()
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

void UWeaponComponent::OnRep_Reload()
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

void UWeaponComponent::SimulateWeaponFire()
{
	if (MuzzleFX)
	{
		MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh, MuzzleAttachPoint);
	}

	//if (!bLoopedFireAnim || !bPlayingFireAnim)
	if (!bPlayingFireAnim)
	{
		PlayWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	if (bLoopedFireSound)
	{
		if (!FireAC)
		{
			FireAC = PlayWeaponSound(FireLoopSound);
		}
	}
	else
	{
		PlayWeaponSound(FireSound);
	}
}

void UWeaponComponent::StopSimulatingWeaponFire()
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

void UWeaponComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponComponent, MyPawn);

	DOREPLIFETIME_CONDITION(UWeaponComponent, BurstCounter, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UWeaponComponent, bPendingReload, COND_SkipOwner);
}

USkeletalMeshComponent* UWeaponComponent::GetWeaponMesh() const
{
	return Mesh;
}

bool UWeaponComponent::IsEquipped() const
{
	return bIsEquipped;
}

bool UWeaponComponent::IsAttachedToPawn() const
{
	return bIsEquipped || bPendingEquip;
}

EWeaponState2 UWeaponComponent::GetCurrentState() const
{
	return CurrentState;
}

float UWeaponComponent::GetEquipStartedTime() const
{
	return EquipStartedTime;
}

float UWeaponComponent::GetEquipDuration() const
{
	return EquipDuration;
}
