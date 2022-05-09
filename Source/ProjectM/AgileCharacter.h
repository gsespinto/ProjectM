// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "MyEnums.h"
#include "AgileCharacter.generated.h"

class AHookPoint;
class UAnimMontage;
class UCableComponent;

UCLASS(config = Game)
class AAgileCharacter : public APlayerCharacter
{
	GENERATED_BODY()

	// _____COMPONENTS_____
public:
	AAgileCharacter();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, Category = "Hook")
		UCableComponent* Rope = nullptr; // Cabble component rope reference
	UPROPERTY(EditDefaultsOnly, Category = "Hook")
		UStaticMeshComponent* Hook = nullptr; // Rope end mesh reference

	virtual void TakeDamage(float Amount) override;

protected:
	virtual void Jump() override;
	void StopJumping();

	/** Called for forwards/backward input */
	virtual void MoveForward(float Value) override;

	/** Called for side to side input */
	virtual void MoveRight(float Value) override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void BeginPlay() override;



	// _____INVENTORY_____
protected:
	virtual void ItemAction() override;
	virtual void PlaceAction() override;


	// ______HOOK_____
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float DetectionDistance = 2000.0f; // Raycast distance to detect possible grapple positions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float GrappleThrowDistance = 500.0f; // Distance from which the player can grapple

	UFUNCTION(BlueprintCallable)
		void SetRopeVisibility(bool bVisible); // Sets rope visibility for grapple start or end, called from animation notify

private:
	AHookPoint* CurrentHookPoint;

	void ActivateHookPoint(AHookPoint* TargetHookPoint);
	void DeactivateHookPointRef();
	void CheckHook(); // Check for pullable and grappable objects

	FVector GrappleDestination; // End location of grapple
	FVector GrapplePointPosition; // Position to which the rope end will go to
	FVector StartingPosition; // Player Starting position of grapple (Change it to be when player does the grapple action)

	float InitialGravityScale = 2.2f; // Initial gravity scale to reset to when grapple has ended
	float RopeBaseLength; // Base length of of the rope

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Hook")
		float MinDetectionDot = 0.7f; // Min dot product to consider that a hook point is at center screen



	// _____GRAPPLING______
public:
	UFUNCTION(BlueprintCallable, Category = "Grapple")
		void StartGrapplingMovement(); // Starts player grapple movement, called from animation notify
	UFUNCTION(BlueprintCallable, Category = "Grapple")
		void ResetGrappleMovement(); // Ends player grapple movement, called from animation notify

private:
	void GrappleAction(); // Called when the player gives the grapple input
	void GrapplingMovement(); // Handles player movement when grappling
	void MoveGrappleRope(); // Handles rope and rope end movement when grappling

	UPROPERTY(EditAnywhere, Category = "Grapple")
		UAnimMontage* GroundGrappleMontage; // Animation montage of grappling when grounded (has notifies used to trigger grapple states)
	UPROPERTY(EditAnywhere, Category = "Grapple")
		UAnimMontage* AirGrappleMontage; // Animation montage of grappling when mid air (has notifies used to trigger grapple states)	

	bool bIsGrappling = false; // Performing grapple
	bool bMovingWithGrapple; // Moving himself using grapple
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
		bool bInGrapplingAnimation; // In middle of grapple animation

	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
		UCurveFloat* GroundSpeedCurve; // Float curve to determine speed of movement to grapple destination when starting from the ground
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
		UCurveFloat* AirSpeedCurve; // Float curve to determine speed of movement to grapple destination when starting mid air

	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
		UCurveFloat* GroundHeightOffsetCurve; // Float curve to determine the height offset from the grapple destination when starting from the ground
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
		UCurveFloat* AirHeightOffsetCurve; // Float curve to determine the height offset from the grapple destination when starting mid air

	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
		UCurveFloat* GroundRopeLength; // Float curve to determine the length of the rope when starting the grapple from the ground
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
		UCurveFloat* AirRopeLength; // Float curve to determine the length of the rope when starting the grapple from mid air

	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
		UCurveFloat* GroundRopePosition; // Float curve to determine the offset to the GrapplePointPosition of the rope end when starting the grapple from the ground
	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
		UCurveFloat* AirRopePosition; // Float curve to determine the offset to the GrapplePointPosition of the rope end when starting the grapple from mid air

	UPROPERTY(EditDefaultsOnly, Category = "Grapple")
		float GrappleLandingOffset = 50.0f; // Z grapple landing offset



	//______PULLING_____
private:
	void PullAction(); // Handles pull input
	void PullingMovement(float DeltaTime); // Handles target and rope pulling movement towards the player
	void ResetPullMovement(); // Reset pulling movement, return to normal state
	void MovePullRope(float DeltaTime); // Handles movement of rope until it reaches the pull target
	void SetThrowTarget();
	void EndPull();

	UPROPERTY(EditAnywhere, Category = "Pull")
		UAnimMontage* GroundPullMontage; // Animation montage of grappling when grounded (has notifies used to trigger grapple states)
	UPROPERTY(EditAnywhere, Category = "Pull")
		UAnimMontage* AirPullMontage; // Animation montage of grappling when mid air (has notifies used to trigger grapple states)

	UPROPERTY(EditAnywhere)
		bool bCanPullWhileFalling = false;

	bool bMovingWithPull; // Moving object with pull action
	bool bIsPulling; // Is perfomaning pull action

	UPROPERTY(EditAnywhere, Category = "Pull")
		float PullSpeed = 1000.0f; // Speed to pull the object towards the player
	UPROPERTY(EditAnywhere, Category = "Pull")
		float PullMinDistance = 1000.0f; // Speed to pull the object towards the player
	UPROPERTY(EditAnywhere, Category = "Pull")
		float ThrowForce = 1500.0f; // Force magnitude at which the object will be thrown at the end of the pull

	UPROPERTY(EditAnywhere, Category = "Pull")
		float PullForwardOffset = 100.0f; // Forward offset of the pull movement

	UStaticMeshComponent* PullActorRef; // Mesh Reference of the object that will be pulled
	FVector PullOffset; // Offset between the hook impact point and the pull object

	AActor* ThrowTarget; // Target actor towards which the pulled object will be thrown

	UPROPERTY(EditAnywhere, Category = "Pull", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float MinThrowTargetDot = 0.7f; // Min dot product to consider that a throw target is at center screen
	UPROPERTY(EditAnywhere, Category = "Pull", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
		float ThrowDotProductThreshold = -1.0f; // Min dot product to consider that the camera as moved enough to justify the object being thrown

	UPROPERTY(EditAnywhere, Category = "Pull")
		float ThrowOffset = 0.0f; // Z axis throw offset value
	UPROPERTY(EditAnywhere, Category = "Pull", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float ThrowCurrentVelocitySizeThreshold = 0.1f; // Max velocity magnitude to consider that the pull object is standing still
	UPROPERTY(EditAnywhere, Category = "Pull", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float ThrowCurrentVelocityEffect = 0.0f; // Percentage of how much the pull velocity will effect the throw force

	


	// _____GRAPPLE_ATTACK_____
public:
	UFUNCTION(BlueprintCallable, Category = "Grapple Attack")
		void StartGrapplingAttackMovement(); // Starts player grapple movement, called from animation notify
	UFUNCTION(BlueprintCallable, Category = "Grapple Attack")
		void ResetGrappleAttack(); // Ends player grapple movement, called from animation notify

	UFUNCTION(BlueprintCallable, Category = "Grapple Attack")
		void DealGrappleAttackDamage(); // Activate melee trigger box
	UPROPERTY(BlueprintReadOnly)
		float CurrentGrappleAttackCooldown = 0.0f;

private:
	UPROPERTY(EditAnywhere, Category = "Grapple Attack")
		float GrappleDamage = 35.0f;
	AActor* GrappleAttackTarget;

	void SetGrappleAttackDestination();

	void GrappleAttackAction(); // Called when the player gives the grapple input
	void BeginGrappleAttack(float DeltaSeconds); // Ticks grapple attack cooldown and sets initial values
	void GrappleAttackMovement(); // Handles player movement when grappling
	void MoveGrappleAttackRope(); // Handles rope and rope end movement when grappling

	UPROPERTY(EditAnywhere, Category = "Grapple Attack")
		UAnimMontage* GroundGrappleAttackMontage; // Animation montage of grappling when grounded (has notifies used to trigger grapple states)
	UPROPERTY(EditAnywhere, Category = "Grapple Attack")
		UAnimMontage* AirGrappleAttackMontage; // Animation montage of grappling when mid air (has notifies used to trigger grapple states)	

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
		bool bIsGrappleAttacking = false; // Performing grapple
	bool bMovingWithGrappleAttack; // Moving himself using grapple

	UPROPERTY(EditDefaultsOnly, Category = "Grapple Attack")
		UCurveFloat* GroundAttackSpeedCurve; // Float curve to determine speed of movement to grapple destination when starting from the ground
	UPROPERTY(EditDefaultsOnly, Category = "Grapple Attack")
		UCurveFloat* AirAttackSpeedCurve; // Float curve to determine speed of movement to grapple destination when starting mid air

	UPROPERTY(EditAnywhere, Category = "Grapple Attack")
		float GrappleAttackForwardOffset = 50.0f;
	FVector GrappleAttackOffset;
	UPROPERTY(EditDefaultsOnly, Category = "Grapple Attack")
		UCurveFloat* GroundAttackHeightOffsetCurve; // Float curve to determine the height offset from the grapple destination when starting from the ground
	UPROPERTY(EditDefaultsOnly, Category = "Grapple Attack")
		UCurveFloat* AirAttackHeightOffsetCurve; // Float curve to determine the height offset from the grapple destination when starting mid air

	UPROPERTY(EditDefaultsOnly, Category = "Grapple Attack")
		UCurveFloat* GroundAttackRopeLength; // Float curve to determine the length of the rope when starting the grapple from the ground
	UPROPERTY(EditDefaultsOnly, Category = "Grapple Attack")
		UCurveFloat* AirAttackRopeLength; // Float curve to determine the length of the rope when starting the grapple from mid air

	UPROPERTY(EditDefaultsOnly, Category = "Grapple Attack")
		UCurveFloat* GroundAttackRopePosition; // Float curve to determine the offset to the GrapplePointPosition of the rope end when starting the grapple from the ground
	UPROPERTY(EditDefaultsOnly, Category = "Grapple Attack")
		UCurveFloat* AirAttackRopePosition; // Float curve to determine the offset to the GrapplePointPosition of the rope end when starting the grapple from mid air

	bool bQueuedGrappleAttack; // Is the grapple attack ability set to be triggered whenever it's possible

	UPROPERTY(EditAnywhere, Category = "Grapple Attack")
		float GrappleAttackCooldown = 1.0f;



	// _____DASH_____
public:
	UPROPERTY(BlueprintReadOnly)
		float CurrentDashCooldown = 0.0f;
private:
	void DashAction(); // Handles dash input
	void DashMovement(float DeltaTime); // Handles dash movement
	void StopDash();

	FVector DashDirection;

	// Percentage of the dash velocity that'll remain after it ends
	UPROPERTY(EditAnywhere, Category = "Dash", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float EndAirDashVelocityFactor = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Dash", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float EndGroundDashVelocityFactor = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Dash", meta = (AllowPrivateAccess = "true"))
		bool bIsDashing;

	UPROPERTY(EditAnywhere, Category = "Dash")
		float DashDistance = 1500.0f; // Distance of dash movement
	float CurrentDashDistance;

	UPROPERTY(EditAnywhere, Category = "Dash")
		float DashAcceleration = 20000.0f; // How fast will it reach the target dash speed
	UPROPERTY(EditAnywhere, Category = "Dash")
		float DashSpeed = 2000.0f; // Target dash speed

	UPROPERTY(EditAnywhere, Category = "Dash")
		class UAnimMontage* DashAnimation;

	UPROPERTY(EditAnywhere, Category = "Dash")
		float DashCooldown = 2.0f;

	// Hold normal movement values to return to after dash
	float NormalMaxSpeed;
	float NormalMaxAcc;
};
