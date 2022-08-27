// Fill out your copyright notice in the Description page of Project Settings.


#include "GiantEnemy.h"
#include "Enemy.h"
#include "Components/SceneComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

AGiantEnemy::AGiantEnemy()
{
	SpawnPoint = CreateDefaultSubobject<USceneComponent>("Spawn Point");
	SpawnPoint->SetupAttachment(GetMesh(), TEXT("head"));

	MeleeVisual = CreateDefaultSubobject<UStaticMeshComponent>("Melee Visual");
	MeleeVisual->SetupAttachment(MeleeTrigger);
}

void AGiantEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void AGiantEnemy::MeleeAttackAction()
{
	Super::MeleeAttackAction();
	MeleeVisual->SetVisibility(true);
}

void AGiantEnemy::BeginMeleeAttack()
{
	Super::BeginMeleeAttack();

	if (StompVfx != nullptr)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), StompVfx, GetMesh()->GetSocketLocation(TEXT("ball_r")), GetActorRotation());
	}

}

void AGiantEnemy::EndMeleeAttack()
{
	Super::EndMeleeAttack();

	MeleeVisual->SetVisibility(false);
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

	if (VomitVfx != nullptr)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), VomitVfx, SpawnPoint->GetComponentLocation(), GetActorRotation());
	}
}
