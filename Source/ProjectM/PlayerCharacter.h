// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyStructs.h"
#include "InteractionInterface.h"
#include "PlayerCharacter.generated.h"

class AHookPoint;
class UAnimMontage;
class UCableComponent;

UCLASS(config = Game)
class APlayerCharacter : public ACharacter, public IInteractionInterface
{
	GENERATED_BODY()


	// _____COMPONENTS_____
public:
	APlayerCharacter();

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;
	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly)
		class UStaticMeshComponent* Weapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* MeleeTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* InteractionTrigger;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual void Tick(float DeltaSeconds) override;


	// ______ANIMATION______
public:
	UPROPERTY(BlueprintReadWrite)
		bool bUpperBodyMontage;
	UFUNCTION(BlueprintCallable)
		void BeginUpperBoddyMontage();
	UFUNCTION(BlueprintCallable)
		void EndUpperBoddyMontage();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void RagdollStart();



	// ______INVENTORY______
protected:

	virtual void Jump();

	void StopJumping();

	/** Called for forwards/backward input */
	virtual void MoveForward(float Value);

	/** Called for side to side input */
	virtual void MoveRight(float Value);

	virtual void AddControllerYawInput(float Value) override;
	virtual void AddControllerPitchInput(float Value) override;

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void BeginPlay() override;

	class UVenariGameInstance* GameInstance;
	


	// _____ATTACK_____
public:
	UPROPERTY(BlueprintReadWrite)
		bool bContinueCombo; // Has given input to continue combo
	bool bInAttackAnimation = false; // Is in an attack animation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int CurrentAttackString = 0; // Current amount of attacks hit during combo

	void QueueSpecialAttack();

	bool bQueuedSpecialAttack; // Has a special attack triggered to start right after current attack animation

	UFUNCTION(BlueprintCallable)
		void BeginAttack(); // Activate melee trigger box
	UFUNCTION(BlueprintCallable)
		void EndAttack(); // Deactivate melee trigget box
	UFUNCTION(BlueprintCallable)
		virtual void EndAttackAnimation(); // Check if should end attacks or continue combo, called at the end of an attack animation

	UFUNCTION(BlueprintCallable)
		void SetCanCombo(bool _CanCombo); // Enable and disable input to continue combos
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int CurrentCombo = 0; // Current combo string
	bool bEndingCombo = false; // Is running the timer to end combo string?

	UFUNCTION(BlueprintCallable)
		virtual void TakeDamage(float Amount); // Handle how character recieves damage

protected:
	virtual void MeleeAttackAction(); // Melee attack input

	UFUNCTION()
		virtual void OnMeleeBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere)
		float Damage = 25.0f; // Damage delt at each attack

	void ContinueCombo(); // Move on to next attack animation
	virtual	void StopCombo(); // Stop current attack animation, reset values
	bool bInCombo;

	int CurrentAnimation = 0; // Current animation index from the attack animations array

	virtual void PlayAttackMontage(int Index); // Play given animation from animation attack array
	virtual void StopAttackMontage(int Index, float StopTime); // Stop given animation from animation attack array
	UPROPERTY(EditAnywhere, Category = "Melee")
		float MeleeAnimationStopSpeed = 0.5f; // Speed at which the attack animations are played

	UPROPERTY(EditAnywhere, Category = "Melee")
		UAnimMontage* HitMontage;
	bool bCanCombo = false; // Can give input to continue combo string

private:
	UPROPERTY(EditAnywhere, Category = "Melee")
		TArray<UAnimMontage*> AttackMontages; // Animation montage of grappling when mid air (has notifies used to trigger grapple states)

	UPROPERTY(EditAnywhere)
		bool bLimitedCombo = false; // Combo will not continue after the last animation of the attack montages array

	UPROPERTY(EditAnywhere)
		float timeToStopAttackStreak = 1.0f; // Amount of seconds after which the combo will end
	float currentTimeToStopAttackStreak;
	void TickStopAttackStreak(float DeltaTime); // Demish ending combo timer, after which it'll stop the combo



	// _____POSSESS_____
public:
	virtual void OnInteractionCPP(APlayerCharacter* Player) override; // Start possessing this character
	void StartPossession(APlayerCharacter* PossessionTarget); // Start possessing target character
	bool bPossessed; // Is this the current player avatar
	UPROPERTY(BlueprintReadOnly)
		bool bPossessing; // Trigger possession camera animation

private:
	APlayerCharacter* PossessTarget = nullptr; // Player Character within view

	// Possession speeds
	UPROPERTY(EditAnywhere)
		float PossessLocSpeed = 25.0f;
	UPROPERTY(EditAnywhere)
		float PossessRotSpeed = 25.0f;
	// Thresholds to finish possession
	UPROPERTY(EditAnywhere)
		float PossessLocThres = 0.01f;
	UPROPERTY(EditAnywhere)
		float PossessRotThres = 0.01f;
	// Camera animation to possess
	void PossessCamMovement(float DeltaSeconds);



	// _____INTERACT_____
public:
	UFUNCTION(BlueprintCallable)
		void Play2DSound(USoundBase* Sound, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f);

protected:
	UFUNCTION()
		virtual void OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnInterationBoxOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UObject* CurrentInteractable;
	void CheckCurrentInteractable(UObject* Interactable);

	virtual void InteractAction(); // Interact with the interactable closer to the player

private:
	void UseInteractable(UObject* InteractableObj);
	void EnableInteractable(UObject* InteractableObj);
	void DisableInteractable(UObject* InteractableObj);



	// ______INVENTORY_____
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		bool AddItem(FItemStruct Item, int Quantity); // Adds item to game instance
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		bool RemoveItem(FItemStruct Item, int Quantity); // Removes item to game instance

	UFUNCTION(BlueprintImplementableEvent)
		void SetInventoryVisibility(bool bIsVisible); // Sets the visibility of the inventory tab
	UPROPERTY(BlueprintReadWrite)
		bool bInventoryVisible;
	
	UPROPERTY(BlueprintReadOnly)
		FVector2D RightAnalogInput;

protected:
	virtual void ItemAction(); // Triggers the consumption or placement of an item
	virtual void PlaceAction(); // Activates placed item

private:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float PlacingDotProductThreshold = 0.8f;

	UPROPERTY(EditAnywhere)
		float PlacingDistance = 1000.0f; // Max distance to place an item
	class AItemActor* PlacingActorRef; // Instantiated item that is being placed
	void PlacingItem(); // Sets the location and rotation of the item that is being placed



	// _____NOTEBOOK_____
protected:
	UFUNCTION(BlueprintImplementableEvent)
		void SetNotebookVisibility(bool bIsVisible); // Sets the visibility of the notebook tab
	bool bIsNotebookVisible;

private:
	void TickNotebookVisibility();
};