// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

class UAnimMontage;
UCLASS()
class PROJECTM_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* MeleeTrigger;

	UFUNCTION(BlueprintCallable)
		void UpdateWalkSpeed(float Value);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent)
		void Knockback(FVector Force);

	UFUNCTION(BlueprintCallable)
		void TakeDamage(float Amount);

	UFUNCTION(BlueprintCallable)
		virtual void MeleeAttackAction();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
		virtual void BeginMeleeAttack();

	UFUNCTION(BlueprintCallable)
		virtual void EndMeleeAttack();

private:
	UFUNCTION()
		virtual void OnMeleeBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditDefaultsOnly)
		UAnimMontage* MeleeAttackAnimation;

	UPROPERTY(EditDefaultsOnly)
		UAnimMontage* DamageAnimation;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
		float MeleeDamage = 10.0f;
};
