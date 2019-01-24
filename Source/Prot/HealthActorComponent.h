// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/ObjectMacros.h"
#include "Components/TextRenderComponent.h"
#include "HealthActorComponent.generated.h"


/** Adding an ability to assign delegates that will be called
 *  when the owner of the component is dead */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeathSignature, AActor*, DeadActor);


/** Handles health-damage system */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROT_API UHealthActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthActorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Gets replication properties, i.e., Health in our case */
	virtual void GetLifetimeReplicatedProps(TArray < class FLifetimeProperty > & OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

/// PARAMETERS
protected:
	/** Current Health value */
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=OnRep_Health, Category = "Game|Health")
	float Health;

	/** Maximum value of Health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Health")
	float MaxHealth;

	/** Text render component - used instead of UMG, to keep the tutorial short */
	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* CharText;

	/** States has Death happened or has not */
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=OnRep_IsDead, Category = "Game|Health")
	bool bIsDead;

	/** Called when Health is equal or below 0 */
	UPROPERTY(BlueprintAssignable, Category = "Game|Health")
	FDeathSignature OnDeath;

/// FUNCTIONALITY
protected:
	/** Checks if Death happened */
	virtual void CheckDeath();

public:
	/** Sets new current value of Health */
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	void SetHealthValue(const float NewHealthValue);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetHealthValue(const float NewHealthValue);

	/** Decreases current value of Health */
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	void DecreaseHealthValue(const float DecreaseValue);

	/** Inreases current value of Health */
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	void IncreaseHealthValue(const float IncreaseValue);

	UFUNCTION(BlueprintCallable)
	void Die();

	/** Sets values to "alive" condition and sets Health to given level */
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	void BringToLife(const float NewHealthValue);

public:
	/** Gets current value of Health */
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	float GetHealthValue() const { return Health; }

	/** Tells if dead */
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	bool IsDead() { return bIsDead; }

	/** Gets maximum health */
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	bool GetMaxHealth() { return MaxHealth; }

	void UpdateCharText();

	private:
	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_IsDead();

};
