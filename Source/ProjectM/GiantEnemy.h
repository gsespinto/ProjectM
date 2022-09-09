// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "GiantEnemy.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTM_API AGiantEnemy : public AEnemy
{
	GENERATED_BODY()

public:
	AGiantEnemy();

	virtual void MeleeAttackAction() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* MeleeVisual;

	UFUNCTION(BlueprintCallable)
		virtual void ProjectileAttackAction();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginMeleeAttack() override;
	virtual void EndMeleeAttack() override;

	UFUNCTION(BlueprintCallable)
		virtual void ProjectileAttack();

private:
	UFUNCTION(BlueprintCallable)
		void SpawnAction();

 	UFUNCTION(BlueprintCallable)
		void SpawnMinions();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Minions", meta = (AllowPrivateAccess = "true"))
		class USceneComponent* SpawnPoint;

	UPROPERTY(EditAnywhere, Category="Minions")
		UAnimMontage* SpawnMontage;

	UPROPERTY(EditAnywhere, Category = "Minions")
		TSubclassOf<class AEnemy> MinionClass;

	UPROPERTY(EditAnywhere, Category="Minions")
		uint32 MinMinionsSpawn;

	UPROPERTY(EditAnywhere, Category="Minions")
		uint32 MaxMinionsSpawn;

	UPROPERTY(EditAnywhere, Category = "Minions")
		class UNiagaraSystem* VomitVfx;

	UPROPERTY(EditAnywhere, Category = "Melee")
		class UNiagaraSystem* StompVfx;

	UPROPERTY(EditAnywhere, Category = "Projectile")
		TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category="Projectile")
		UAnimMontage* ProjectileMontage;

	UPROPERTY(EditAnywhere, Category = "Projectile")
		TSubclassOf<AActor> VisualsClass;

	UPROPERTY(EditAnywhere)
		TArray<USoundBase*> VomitSfx;

	UPROPERTY(EditAnywhere)
		TArray<USoundBase*> StompSfx;

	FVector ProjectileTarget;
	AActor* CurrentVisuals;

	void VomitFX();
};
