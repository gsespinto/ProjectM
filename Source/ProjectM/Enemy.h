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
		virtual void TakeDamage(float Amount);

	UFUNCTION(BlueprintCallable)
		virtual void MeleeAttackAction();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
		virtual void BeginMeleeAttack();

	UFUNCTION(BlueprintCallable)
		virtual void EndMeleeAttack();

	UPROPERTY(EditAnywhere, Category = "SFX")
		class USoundAttenuation* SoundAttenuation;

	UFUNCTION(BlueprintCallable)
		bool IsInMeleeCooldown();

private:
	UFUNCTION()
		virtual void OnMeleeBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditDefaultsOnly)
		TArray<UAnimMontage*> MeleeAttackAnimations;

	UPROPERTY(EditDefaultsOnly)
		TArray<UAnimMontage*> DeathAnimations;

	UPROPERTY(EditDefaultsOnly)
		UAnimMontage* DamageAnimation;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
		float MeleeDamage = 10.0f;

	UPROPERTY(EditAnywhere, Category = "SFX")
		TArray<USoundBase*> MeleeSfx;

	UPROPERTY(EditAnywhere, Category = "SFX")
		TArray<USoundBase*> DamagedSfx;

	UPROPERTY(EditAnywhere, Category = "SFX")
		TArray<USoundBase*> DeathSfx;

	TArray<UMaterialInstanceDynamic*> DynamicMaterials;
	
	void DeactivateAI();

	UPROPERTY(EditAnywhere)
		float MeleeCooldown = 2.0f;
	float CurrentMeleeCooldown;

	void TickMeleeCooldown(float DeltaTime);
};
