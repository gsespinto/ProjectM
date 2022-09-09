// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
	// ...
	
}

// Decrease CurrentHP by given amount
void UHealthComponent::TakeDamage(float Amount)
{
	Amount = FMath::Abs(Amount);
	CurrentHP -= Amount;
	CurrentHP = FMath::Clamp(CurrentHP, 0.0f, MaxHP);
}

// Increase CurrentHP by given amount
void UHealthComponent::Heal(float Amount)
{
	Amount = FMath::Abs(Amount);
	CurrentHP += Amount;
	CurrentHP = FMath::Clamp(CurrentHP, 0.0f, MaxHP);
}

bool UHealthComponent::IsDead()
{
	return CurrentHP <= 0.0f;
}

float UHealthComponent::GetHPRatio()
{
	return CurrentHP / MaxHP;
}

