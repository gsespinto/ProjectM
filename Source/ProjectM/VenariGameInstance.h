// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyStructs.h"
#include "MyEnums.h"
#include "VenariGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTM_API UVenariGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FItemStruct> InventoryData;

	UPROPERTY(BlueprintReadWrite)
		int EquippedItemIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FQuestInfo CurrentQuest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> DefeatedMonsters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EPlayerCharacter CurrentCharacter;
};
