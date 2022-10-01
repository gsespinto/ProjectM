// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Components/AudioComponent.h"

SoundManager::SoundManager()
{
}

SoundManager::~SoundManager()
{
}

bool SoundManager::PlayRandomSoundAtLocation(const UObject* WorldObjectContext, TArray<class USoundBase*> Sounds, FVector Location, USoundAttenuation* AttenuationSettings)
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
	if (AttenuationSettings != nullptr)
		UGameplayStatics::SpawnSoundAtLocation(WorldObjectContext, Sounds[Index], Location, FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, AttenuationSettings);
	else
		UGameplayStatics::SpawnSoundAtLocation(WorldObjectContext, Sounds[Index], Location);

	return true;
}

bool SoundManager::PlayRandomSoundAttached(TArray<class USoundBase*> Sounds, USceneComponent* AttachToComponent, FName Socket, USoundAttenuation* AttenuationSettings)
{
	if (AttachToComponent)
	{
		return false;
	}

	if (Sounds.Num() <= 0)
	{
		return false;
	}

	int Index = FMath::RandRange(0, Sounds.Num() - 1);

	if (AttenuationSettings != nullptr)
		UGameplayStatics::SpawnSoundAttached(Sounds[Index], AttachToComponent, Socket, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, false, 1.0f, 1.0f, 0.0f, AttenuationSettings);
	else
		UGameplayStatics::SpawnSoundAttached(Sounds[Index], AttachToComponent, Socket, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset);

	return true;
}

bool SoundManager::PlayRandomSoundAudioComponent(UAudioComponent* AudioComponent, TArray<class USoundBase*> Sounds)
{
	if (AudioComponent == nullptr)
	{
		return false;
	}

	if (Sounds.Num() <= 0)
	{
		return false;
	}

	int Index = FMath::RandRange(0, Sounds.Num() - 1);
	AudioComponent->Sound = Sounds[Index];
	AudioComponent->Play();

	return true;
}
