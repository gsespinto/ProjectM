// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DestructableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDestructableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTM_API IDestructableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(Category = "Destructable Interface", BlueprintNativeEvent, BlueprintCallable)
		void OnDestruction(class APlayerCharacter* PlayerCharacter);

	virtual void OnDistructionCPP(class APlayerCharacter* PlayerCharacter);
};
