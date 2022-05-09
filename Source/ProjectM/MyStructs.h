// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyEnums.h"
#include "Engine/DataTable.h"
#include "MyStructs.generated.h"

USTRUCT(Blueprintable)
struct FItemStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		FName Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		EItemType Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		class UTexture2D* Icon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		class UStaticMesh* Model;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		int Quantity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		bool bStackable = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		TSubclassOf<class AItemActor> ItemActor;
};

USTRUCT(Blueprintable)
struct FMonsterHints : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint")
		FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint")
		EFootprintsHint FootprintsHint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint")
		EFurHint FurHint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint")
		EResidueHint ResidueHint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint")
		class UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint")
		FString Weaknesses;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint")
		FString Strengths;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint")
		FString Description;
};

USTRUCT(Blueprintable)
struct FQuestInfo : public FTableRowBase
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
		FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
		FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
		class UTexture2D* Icon;
};