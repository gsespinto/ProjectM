// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "MyEnums.h"
#include "BerserkerCharacter.generated.h"

/**
 * 
 */

class UAnimMontage;

UCLASS()
class PROJECTM_API ABerserkerCharacter : public APlayerCharacter
{
	GENERATED_BODY()
	
		// _____COMPONENTS_____
public:
	ABerserkerCharacter();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void Jump() override;

	/** Called for forwards/backward input */
	virtual void MoveForward(float Value) override;

	/** Called for side to side input */
	virtual void MoveRight(float Value) override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;



	// _____MELEE_____
public:
	virtual void TakeDamage(float Amount) override;

protected:
	virtual void OnMeleeBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;



	// _____INVENTORY_____
protected:
	virtual void ItemAction() override;
	virtual void PlaceAction() override;



	// _____SHOULDER_BASH_____
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "ShoulderBash")
		class UBoxComponent* BashTrigger;

	UFUNCTION(BlueprintCallable, Category = "ShoulderBash")
		void BeginBashAttack(); // Activate shoulder bash trigger box
	UFUNCTION(BlueprintCallable, Category = "ShoulderBash")
		void EndBashAttack(); // Deactivate shoulder bash trigget box

	UFUNCTION(BlueprintCallable, Category = "ShoulderBash")
		void BeginBashMovement(); // Begin bash movement
	UFUNCTION(BlueprintCallable, Category = "ShoulderBash")
		void EndShoulderBash(); // End shoulder bash movement and reset values

	UPROPERTY(BlueprintReadOnly)
		float CurrentBashCooldown = 0.0f;

private:
	UPROPERTY(EditAnywhere, Category = "ShoulderBash")
		float BashDistance = 250.0f; // Max distance of shoulder bash
	float CurrentBashDistance;

	UPROPERTY(EditAnywhere, Category = "ShoulderBash")
		float BashKnockbackForce = 100.0f; // Force magnitude that'll be applied to the shoulder bash enemy
	UPROPERTY(EditAnywhere, Category = "ShoulderBash", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float BashDistancePercentToKnockback = 0.0f; // Percentage of bash distance that needs to be done to apply knockback to enemies

	// Shoulder bash movement values
	UPROPERTY(EditAnywhere, Category = "ShoulderBash")
		float BashSpeed = 1700.0f;
	UPROPERTY(EditAnywhere, Category = "ShoulderBash")
		float BashAcceleration = 20000.0f;

	UPROPERTY(EditAnywhere, Category = "ShoulderBash")
		float BashDamage = 45.0f;

	UPROPERTY(EditAnywhere)
		bool bFinishBashUponImpact = false; // If true the shoulder bash ends when upon impact with any actor

	// Normal movement values to reset to after the shoulder bash
	float NormalSpeed = 0.0f;
	float NormalAcceleration = 0.0f;

	UPROPERTY(EditAnywhere, Category = "ShoulderBash")
		UAnimMontage* BashMovementAnim;
	UPROPERTY(EditAnywhere, Category = "ShoulderBash")
		UAnimMontage* BashEndAnim;

	UPROPERTY(BlueprintReadWrite, Category = "ShoulderBash", meta = (AllowPrivateAccess = "true"))
		bool bIsBashing = false;
	bool bMoveWithBash = false;
	bool bQueuedBash; // Is the shoulder bash ability set to be triggered whenever it's possible

	void ShoulderBashAction(); // input
	void BeginShoulderBash(); // trigger shoulder bash
	void BashMovement(float DeltaSeconds); // handles ability's movement
	void EndBashMovement(); // ends bash movement and resets values

	FVector BashDirection;

	UFUNCTION()
		void OnShoulderBashBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, Category = "ShoulderBash")
		float BashCooldown = 2.0f;
	void TickBashCooldown(float DeltaSeconds);



	// _____BOOSTS_____
public:
	UPROPERTY(BlueprintReadOnly)
		float CurrentLifeStealCooldown;
	UPROPERTY(BlueprintReadOnly)
		float CurrentBerserkCooldown;

protected:
	UPROPERTY(BlueprintReadWrite)
		bool bUsingLifeSteal = false;
	UPROPERTY(BlueprintReadWrite)
		bool bUsingBerserk = false;

private:
	void UseBoost(EBoostType BoostType);

	UPROPERTY(EditAnywhere, Category = "LifeStealBoost",
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float LifeStealPercent = 0.5f; // Percentage of the damage given that'll be recieve as health points

	UPROPERTY(EditAnywhere, Category = "LifeStealBoost")
		float LifeStealDuration = 2.0f;
	float CurrentLifeStealDuration;

	UPROPERTY(EditAnywhere, Category = "LifeStealBoost")
		float LifeStealCooldown = 2.0f;

	void TickLifeSteal(float DeltaSeconds); // Ticks lifesteal boost duration and cooldown

	UPROPERTY(EditAnywhere, Category = "BerserkBoost")
		float DamageBoost = 10.0f; // Berserk boost's extra damage points
	float OriginalDamage; // Original damage value to reset to
	UPROPERTY(EditAnywhere, Category = "BerserkBoost", 
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float DamageDamping = 0.5f; // Percentage of damage that'll be discarded while using the berserk boost

	UPROPERTY(EditAnywhere, Category = "BerserkBoost")
		float BerserkDuration = 2.0f;
	float CurrentBerserkDuration;

	UPROPERTY(EditAnywhere, Category = "BerserkBoost")
		float BerserkCooldown = 2.0f;

	void TickBerserkBoost(float DeltaSeconds); // Ticks berserk boost duration and cooldown
};
