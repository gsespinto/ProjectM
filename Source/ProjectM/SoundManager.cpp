// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundManager.h"
#include "Kismet/GameplayStatics.h"

SoundManager::SoundManager()
{
}

SoundManager::~SoundManager()
{
}

bool SoundManager::PlayRandomSoundAtLocation(const UObject* WorldObjectContext, TArray<class USoundBase*> Sounds, FVector Location)
{
	if (WorldObjectContext == nullptr)
	{
		return false;
	}

	if (Sounds.Num() <= 0)
	{
		return false;
	}

	int Index = FMath::RandRange(0, Sounds.Num() - 1);
	UGameplayStatics::SpawnSoundAtLocation(WorldObjectContext, Sounds[Index], Location);
	return true;
}
