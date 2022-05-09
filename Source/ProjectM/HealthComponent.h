// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTM_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MaxHP = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float CurrentHP = 0.0f;

	UFUNCTION(BlueprintCallable)
		void TakeDamage(float Amount); // Decrease CurrentHP by given amount
	UFUNCTION(BlueprintCallable)
		void Heal(float Amount); // Increase CurrentHP by given amount

	UFUNCTION(BlueprintCallable)
		bool IsDead(); // Returns true if CurrentHP < 0

protected:
	// Called when the game starts
	virtual void BeginPlay() override;		
};
