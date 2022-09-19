// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class PROJECTM_API SoundManager
{
public:
	SoundManager();
	~SoundManager();

	static bool PlayRandomSoundAtLocation(const UObject* WorldObjectContext, TArray<class USoundBase*> Sounds, FVector Location, class USoundAttenuation* AttenuationSettings = nullptr);
	static bool PlayRandomSoundAudioComponent(class UAudioComponent* AudioComponent, TArray<class USoundBase*> Sounds);
};
