// Fill out your copyright notice in the Description page of Project Settings.


#include "GiantEnemy.h"
#include "Enemy.h"
#include "Components/SceneComponent.h"

AGiantEnemy::AGiantEnemy()
{
	SpawnPoint = CreateDefaultSubobject<USceneComponent>("Spawn Point");
	SpawnPoint->SetupAttachment(GetMesh(), TEXT("head"));
}

void AGiantEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void AGiantEnemy::SpawnAction()
{
	if (SpawnMontage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Missing spawn montage on Giant Enemy!"));
		return;
	}

	GetMesh()->GetAnimInstance()->Montage_Play(SpawnMontage);
}

void AGiantEnemy::SpawnMinions()
{
	if (MinionClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Missing minion class on Giant Enemy!"));
		return;
	}

	int Amount = FMath::RandRange((int)MinMinionsSpawn, (int)MaxMinionsSpawn);

	for (int i = 0; i < Amount; i++)
	{
		GetWorld()->SpawnActor<ACharacter>(MinionClass, SpawnPoint->GetComponentLocation(), GetActorRotation(), FActorSpawnParameters());
	}
}
