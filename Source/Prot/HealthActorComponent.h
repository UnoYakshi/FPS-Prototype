// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/ObjectMacros.h"
#include "HealthActorComponent.generated.h"

//Adding an ability to assign delegates that will be called
//when the owner of the component is dead
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeathSignature, AActor*, DeadActor);

//Component for additng health to the game character
UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROT_API UHealthActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthActorComponent();

protected:
	virtual void BeginPlay() override;

	//Current Health value
	UPROPERTY(Replicated, EditDefaultsOnly)
	float Health;

	//Checks if Death happened
	virtual void CheckDeath();

	//States has Death happened or has not
	UPROPERTY(Replicated, EditDefaultsOnly)
	bool bIsDead;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Maximum value of Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth;

	//Gets current value of Health
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	float GetHealthValue() const { return Health; }

	//Called when Health is equal or below 0
	UPROPERTY(BlueprintAssignable, Category = "Game|Health")
	FDeathSignature OnDeath;

	//Sets new current value of Health
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	void SetHealthValue(float NewHealthValue);

	//Decreases current value of Health
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	void DecreaseHealthValue(float DecreaseValue) { SetHealthValue(Health - DecreaseValue); }

	//Inreases current value of Health
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	void IncreaseHealthValue(float IncreaseValue) { SetHealthValue(Health + IncreaseValue); }

	//Sets values to "alive" condition
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	void BringToLife() { bIsDead = false; SetHealthValue(MaxHealth); }
	
	//Sets values to "alive" condition and sets Health to given level
	void BringToLife(float NewHealthValue) { bIsDead = false; SetHealthValue(NewHealthValue); }

	//Tells if dead
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	bool IsDead() { return bIsDead; }

	//Gets maximum health
	UFUNCTION(BlueprintCallable, Category = "Game|Health")
	bool GetMaxHealth() { return MaxHealth; }

	//Gets replication properties, i.e. Health in our case
	virtual void GetLifetimeReplicatedProps(TArray < class FLifetimeProperty > & OutLifetimeProps) const override;
};