// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTM_API IInteractionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// BLUEPRINTS
	UFUNCTION(Category = "My Interface", BlueprintNativeEvent, BlueprintCallable)
		void OnInteraction(class APlayerCharacter* PlayerCharacter);

	UFUNCTION(Category = "My Interface", BlueprintNativeEvent, BlueprintCallable)
		void OnEnable(); // Show visual cueue
	UFUNCTION(Category = "My Interface", BlueprintNativeEvent, BlueprintCallable)
		void OnDisable(); // Hide visual Cue

	// CPP
	virtual void OnInteractionCPP(class APlayerCharacter* PlayerCharacter);
	virtual	void OnEnableCPP(); // Show visual cueue
	virtual	void OnDisableCPP(); // Hide visual Cue
};
