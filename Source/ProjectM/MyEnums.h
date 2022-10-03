// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
UENUM()
enum class EBoostType : uint8 {
	LIFESTEAL = 0,
	BERSERK = 1,
};

UENUM()
enum class EHookType : uint8 {
	GRAPPABLE = 0,
	PULLABLE = 1,
	ENEMY = 2,
	NONE = 3,
};

UENUM(BlueprintType)
enum class EDialogueState : uint8 {
	SPEAK = 0 UMETA(DisplayName = "SPEAK"),
	REPLY = 1 UMETA(DisplayName = "REPLY"),
};

UENUM(BlueprintType)
enum class EItemType : uint8 {
	CONSUMABLE = 0 UMETA(DisplayName = "Consumable"),
	PLACEABLE = 1 UMETA(DisplayName = "Placeable"),
};

UENUM(BlueprintType)
enum class EFootprintsHint : uint8 {
	UNKNOWN = 0 UMETA(DisplayName = "???"),
	HOOVES = 1 UMETA(DisplayName = "Hooves"),
	CANINE = 2 UMETA(DisplayName = "Canine"),
	BIRD = 3 UMETA(DisplayName = "Bird"),
	INSECT = 4 UMETA(DisplayName = "Insect"),
	HUMANOID = 5 UMETA(DisplayName = "Humanoid"),
	LINES = 6 UMETA(DisplayName = "Lines"),
};

UENUM(BlueprintType)
enum class EFurHint : uint8 {
	UNKNOWN = 0 UMETA(DisplayName = "???"),
	DEAD_SKIN = 1 UMETA(DisplayName = "Dead Skin"),
	HAIR = 2 UMETA(DisplayName = "Hair"),
	FEATHERS = 3 UMETA(DisplayName = "Feathers"),
	SKIN = 4 UMETA(DisplayName = "Skin"),
	FUR = 5 UMETA(DisplayName = "Fur"),
	SCALES = 6 UMETA(DisplayName = "Scales"),
};

UENUM(BlueprintType)
enum class EResidueHint : uint8 {
	UNKNOWN = 0 UMETA(DisplayName = "???"),
	SLIME = 1 UMETA(DisplayName = "Slime"),
	BROWN_POOP = 2 UMETA(DisplayName = "Mineral"),
	WHITE_POOP = 3 UMETA(DisplayName = "Blood"),
	ACID = 4 UMETA(DisplayName = "Acid"),
};

UENUM(BlueprintType)
enum class EPlayerCharacter : uint8 {
	AGILE = 0 UMETA(DisplayName = "Agile"),
	BERSERKER = 1 UMETA(DisplayName = "Berserker"),
};