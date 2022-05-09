// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "GameFramework/Actor.h"
#include "MyEnums.h"
#include "HookPoint.generated.h"

class APlayerCharacter;

UCLASS()
class PROJECTM_API AHookPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHookPoint();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void Activate(AAgileCharacter* PawnRef); // Show hook point
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void Deactivate(); // Hide hook point

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void CheckDistanceFromPlayer(); // Check if player is close enough to use this hookpoint
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void Use(); // Trigger visual cueue

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		USceneComponent* GetLandingPoint(); // Return landing point scene component ref
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		bool CanUse(); // Check if the player can use this hook point

	// Type of hook point that'll change visuals
	// If NONE this hook point can't be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EHookType Type = EHookType::NONE;


	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
