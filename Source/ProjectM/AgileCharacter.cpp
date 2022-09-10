// Copyright Epic Games, Inc. All Rights Reserved.

#include "AgileCharacter.h"
#include "PlayerCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "HookPoint.h"
#include "Animation/AnimInstance.h"
#include "CableComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "HealthComponent.h"
#include "Enemy.h"

//////////////////////////////////////////////////////////////////////////
// AAgileCharacter

AAgileCharacter::AAgileCharacter()
{
	// Create rope end hook
	Hook = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hook"));
	Hook->SetupAttachment(RootComponent);
	Hook->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create cable component rope
	Rope = CreateDefaultSubobject<UCableComponent>(TEXT("Rope"));
	Rope->AttachTo(GetMesh(), TEXT("hand_l"));
	Rope->SetAttachEndTo(this, Hook->GetFName());
	Rope->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Rope->CableWidth = 2.0f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AAgileCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Set initial gravity to character movement's value
	InitialGravityScale = GetCharacterMovement()->GravityScale;

	// Set initial hook abilities values
	ResetGrappleMovement();
	ResetGrappleAttack();
	ResetPullMovement();

	// Hide rope
	SetRopeVisibility(false);

	// Get normal initial speeds
	NormalMaxSpeed = GetCharacterMovement()->MaxWalkSpeed;
	NormalMaxAcc = GetCharacterMovement()->MaxAcceleration;
}

void AAgileCharacter::ItemAction()
{
	if (bInGrapplingAnimation || bIsPulling || bIsDashing || bIsGrappleAttacking)
		return;

	Super::ItemAction();
}

void AAgileCharacter::PlaceAction()
{
	if (bInGrapplingAnimation || bIsPulling || bIsDashing || bIsGrappleAttacking)
		return;

	Super::PlaceAction();
}

void AAgileCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Check if can pull or grapple
	CheckHook();

	// Grappling
	GrapplingMovement();
	MoveGrappleRope();

	// Grappling Attack
	BeginGrappleAttack(DeltaSeconds);
	GrappleAttackMovement();
	MoveGrappleAttackRope();

	// Pulling
	PullingMovement(DeltaSeconds);
	MovePullRope(DeltaSeconds);

	// DashMovement
	DashMovement(DeltaSeconds);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AAgileCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	// Player Character input
	APlayerCharacter::SetupPlayerInputComponent(PlayerInputComponent);

	// Grapple
	PlayerInputComponent->BindAction("Grapple", IE_Pressed, this, &AAgileCharacter::GrappleAction);

	// Grapple Attack
	PlayerInputComponent->BindAction("Grapple", IE_Pressed, this, &AAgileCharacter::GrappleAttackAction);

	// Pull
	PlayerInputComponent->BindAction("Grapple", IE_Pressed, this, &AAgileCharacter::PullAction);
	PlayerInputComponent->BindAction("Grapple", IE_Released, this, &AAgileCharacter::ResetPullMovement);

	// DashMovement
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AAgileCharacter::DashAction);
}

void AAgileCharacter::MoveForward(float Value)
{
	// Can't move while grappling, pulling or dashing
	if (bInGrapplingAnimation || bIsPulling || bIsDashing || bIsGrappleAttacking)
		return;

	// Store input value for dash
	DashDirection.X = FMath::Sign(Value);

	APlayerCharacter::MoveForward(Value);
}

void AAgileCharacter::MoveRight(float Value)
{
	// Can't move while grappling, pulling or dashing
	if (bInGrapplingAnimation || bIsPulling || bIsDashing || bIsGrappleAttacking)
		return;

	// Store input value for dash
	DashDirection.Y = FMath::Sign(Value);

	APlayerCharacter::MoveRight(Value);
}

void AAgileCharacter::Jump()
{
	// Can't jump while grappling, pulling or dashing
	if (bInGrapplingAnimation || bIsPulling || bIsDashing || bIsGrappleAttacking)
		return;

	APlayerCharacter::Jump();
}

void AAgileCharacter::StopJumping()
{
	ACharacter::StopJumping();
}

// Check for valid grapple or pull position
void AAgileCharacter::CheckHook()
{
	// If player dies, deactivate hook point
	if (HealthComponent->IsDead() || bPossessing || bIsNotebookVisible)
	{
		DeactivateHookPointRef();
		return;
	}

	// Do nothing if hook point ref is in use
	if (bIsGrappling || bIsPulling || bIsGrappleAttacking)
		return;

	// Sphere cast to detected hook points within range of player
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(EObjectTypeQuery::ObjectTypeQuery7);
	TArray<AActor*> ActorsToIgnore;
	TArray<FHitResult> SphereHits;

	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), this->GetActorLocation(), this->GetActorLocation(), DetectionDistance, ObjTypes, false, ActorsToIgnore, EDrawDebugTrace::None, SphereHits, false);
	
	// If no hook points are within range, deactivate hook point ref
	if (SphereHits.Num() <= 0)
	{
		DeactivateHookPointRef();
		return;
	}

	// Check which hook point is closest to center screen
	float HighestDot = MinDetectionDot;
	AHookPoint* DetectedHookPoint = nullptr;

	for (int i = 0; i < SphereHits.Num(); i++)
	{
		FVector CurrentDirection = SphereHits[i].GetActor()->GetActorLocation() - FollowCamera->GetComponentLocation();
		CurrentDirection.Normalize();
		float DotProd = FVector::DotProduct(FollowCamera->GetForwardVector(), CurrentDirection);

		if (DotProd > HighestDot)
		{
			DetectedHookPoint = Cast<AHookPoint>(SphereHits[i].GetActor());
			HighestDot = DotProd;
		}
	}

	// if no hook points are close to the center screen, if the detected hook point cant be used or its hook type is NONE
	// deactivate hook point ref
	if (DetectedHookPoint == nullptr || !DetectedHookPoint->CanUse() || DetectedHookPoint->Type == EHookType::NONE)
	{
		DeactivateHookPointRef();
		return;
	}

	// If the detected hook is for the grapple attack but the ability is in cooldown
	// Deactivate hook point
	if (DetectedHookPoint->Type == EHookType::ENEMY && CurrentGrappleAttackCooldown > 0.0f)
	{
		DeactivateHookPointRef();
		return;
	}

	// Line trace to check if the detected hook point is visible
	FHitResult LineHit;
	GetWorld()->LineTraceSingleByChannel(LineHit, FollowCamera->GetComponentLocation(),
		DetectedHookPoint->GetActorLocation(), ECC_Visibility);

	// If it isn't visible, deactivate hook point ref
	if (LineHit.Actor == nullptr)
	{
		DeactivateHookPointRef();
		return;
	}

	// ((For attack points that are within the mesh of the enemy))
	// If the hit actor isn't the hook point and the hook point is not one of the child actors of the hit target
	// Deactivate current hook point
	TArray<AActor*> ChildActors;
	LineHit.GetActor()->GetAllChildActors(ChildActors);
	if (LineHit.GetActor() != DetectedHookPoint && !ChildActors.Contains(DetectedHookPoint))
	{
		DeactivateHookPointRef();
		return;
	}

	// If it's an enemy hook point
	// Set grapple destination to line trace impact point
	if (DetectedHookPoint->Type == EHookType::ENEMY)
	{
		GrappleAttackOffset = LineHit.ImpactPoint - LineHit.GetActor()->GetActorLocation();
		GrappleAttackTarget = LineHit.GetActor()->GetParentActor();
	}
	
	// If the detected hook point is the same as the current hook point ref, do nothing
	if (DetectedHookPoint == CurrentHookPoint)
		return;


	// Set the current hook point ref as the detected hook point and activate it
	ActivateHookPoint(DetectedHookPoint);
}


// Set current hook point as given target hook point and activate it
void AAgileCharacter::ActivateHookPoint(AHookPoint* TargetHookPoint)
{
	// Deactivate old hook point ref
	DeactivateHookPointRef();

	// Set new one to current and activate it
	CurrentHookPoint = TargetHookPoint;
	CurrentHookPoint->Activate(this);

	// If the current hook point type is PULLABLE, set the pull actor ref as the static mesh of the parent actor
	if (CurrentHookPoint->Type == EHookType::PULLABLE) 
	{
		TArray<UStaticMeshComponent*> Components;
		CurrentHookPoint->GetParentActor()->GetComponents<UStaticMeshComponent>(Components);
		PullActorRef = Components[0];
	}
}

// Deactivate hook point and clear ref
void AAgileCharacter::DeactivateHookPointRef()
{
	if (!CurrentHookPoint)
	{
		return;
	}

	CurrentHookPoint->Deactivate();
	CurrentHookPoint = nullptr;
}

// Called when the player gives the grapple input
void AAgileCharacter::GrappleAction()
{
	// Do nothing if it's possessing another player character or with the notebook open
	if (bPossessing || bIsNotebookVisible)
		return;

	// Do nothing if the player character is dead or there is no valid hookpoint
	if (HealthComponent->IsDead() || !CurrentHookPoint)
		return;

	// If is grappling or can't grapple return
	if (bIsGrappling || CurrentHookPoint->Type != EHookType::GRAPPABLE)
		return;

	// If is player is too far off the grapple point to grapple return
	if (FVector::Distance(GetActorLocation(), CurrentHookPoint->GetActorLocation()) > GrappleThrowDistance)
		return;

	EndUpperBoddyMontage();
	StopDash();

	// Set grapple destination to grapple point landing point with player height offset
	GrappleDestination = CurrentHookPoint->GetLandingPoint()->GetComponentLocation() + FVector::UpVector * GrappleLandingOffset;
	// Set rope end destination
	GrapplePointPosition = CurrentHookPoint->GetActorLocation();

	// Rotate player towards the grapple destination
	FRotator TargetRotation = FRotator::MakeFromEuler(FVector::UpVector * UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), GrappleDestination).Yaw);
	SetActorRotation(TargetRotation);

	// Set rope base length to distance from the player to the grapple destination
	RopeBaseLength = FVector::Distance(GrappleDestination, GetActorLocation());

	// Tick grapple point visual cueu
	// Tick grappling animation
	CurrentHookPoint->Use();
	bInGrapplingAnimation = true;

	// Trigger animation montage depending on initial state (grounded or mid air)
	if (GetCharacterMovement()->IsFalling())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(AirGrappleMontage);
	}
	else
	{
		GetMesh()->GetAnimInstance()->Montage_Play(GroundGrappleMontage);
	}
}

// Handles player movement when grappling
void AAgileCharacter::GrapplingMovement()
{
	// If it's not moving using grapple return
	if (!bMovingWithGrapple)
		return;

	// Get montage position for curves
	UAnimInstance* AnimeInstanceRef = GetMesh()->GetAnimInstance();
	float MontagePosition = AnimeInstanceRef->Montage_GetPosition(AnimeInstanceRef->GetCurrentActiveMontage());
	// Target location of the grapple
	FVector TargetLocation;

	// Set target position to lerp between grapple starting position and destination, with correspondent montage speed curve as alpha
	// Adds height offset as correspondent montage curve
	if (AnimeInstanceRef->GetCurrentActiveMontage() == GroundGrappleMontage)
	{
		TargetLocation = FMath::Lerp(StartingPosition, GrappleDestination, GroundSpeedCurve->GetFloatValue(MontagePosition));
		TargetLocation += FVector::UpVector * GroundHeightOffsetCurve->GetFloatValue(MontagePosition);
	}
	else
	{
		TargetLocation = FMath::Lerp(StartingPosition, GrappleDestination, AirSpeedCurve->GetFloatValue(MontagePosition));
		TargetLocation += FVector::UpVector * AirHeightOffsetCurve->GetFloatValue(MontagePosition);
	}

	// Set player position to target location
	SetActorLocation(TargetLocation);
}

// Starts player grapple movement, called from animation notify
void AAgileCharacter::StartGrapplingMovement()
{
	// Stops player in place, and sets default mode to falling
	GetCharacterMovement()->GravityScale = 0.0f;
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);

	// Sets grapple starting position
	StartingPosition = GetActorLocation();

	// Ticks GrapplingMovement
	bMovingWithGrapple = true;
}

// Ends player grapple movement and resets values
void AAgileCharacter::ResetGrappleMovement()
{
	bMovingWithGrapple = false;
	GetCharacterMovement()->GravityScale = InitialGravityScale;
	bInGrapplingAnimation = false;
	SetRopeVisibility(false);
}

// Handles rope and rope end movement when grappling
void AAgileCharacter::MoveGrappleRope()
{
	if (!bInGrapplingAnimation)
		return;

	// Get current grapple montage position
	UAnimInstance* AnimeInstanceRef = GetMesh()->GetAnimInstance();
	float MontagePosition = AnimeInstanceRef->Montage_GetPosition(AnimeInstanceRef->GetCurrentActiveMontage());

	// Set initial target rope lenght as base value
	float TargetLength = RopeBaseLength;
	// Alpha value of end rope position lerp
	float TargetPosition;
	// Rope end position
	FVector RopeEndPosition;

	// Multiply rope length by correspondent montage float curve length
	// Set position lerp alpha to correspondent montage float curve length
	if (AnimeInstanceRef->GetCurrentActiveMontage() == GroundGrappleMontage)
	{
		TargetLength *= GroundRopeLength->GetFloatValue(MontagePosition);
		TargetPosition = GroundRopePosition->GetFloatValue(MontagePosition);
	}
	else
	{
		TargetLength *= AirRopeLength->GetFloatValue(MontagePosition);
		TargetPosition = AirRopePosition->GetFloatValue(MontagePosition);
	}
	// Set rope length
	Rope->CableLength = TargetLength;

	// Lerp rope end position from hand to end position by target position alpha
	RopeEndPosition = FMath::Lerp(GetMesh()->GetSocketLocation(TEXT("hand_l")), GrapplePointPosition, TargetPosition);
	Hook->SetWorldLocation(RopeEndPosition);
	// Set rope cable end location to same position as rope end mesh (Shouldn't be necessary but bugs you know?)
	Rope->EndLocation = RopeEndPosition;
}

// Called when the player gives the pull input
void AAgileCharacter::PullAction()
{
	// Do nothing if it's possessing another player character or with the notebook open
	if (bPossessing || bIsNotebookVisible)
		return;

	// Do nothing if the player character is dead or there is no valid hookpoint
	if (HealthComponent->IsDead() || !CurrentHookPoint)
		return;

	// Do nothing if can't pull object while falling and is falling
	if (!bCanPullWhileFalling && GetCharacterMovement()->IsFalling())
		return;

	// If is pulling or can't pull return
	if (bIsPulling || CurrentHookPoint->Type != EHookType::PULLABLE || !PullActorRef)
		return;

	// If is player is too far off the point to pull return
	if (FVector::Distance(GetActorLocation(), CurrentHookPoint->GetActorLocation()) > GrappleThrowDistance)
		return;

	EndUpperBoddyMontage();
	StopDash();

	// Set rope end destination
	GrapplePointPosition = CurrentHookPoint->GetActorLocation();

	// Set rope base length to distance from the player to the grapple destination
	RopeBaseLength = FVector::Distance(GrapplePointPosition, GetActorLocation());

	// Tick point visual cueu
	// Tick pulling
	CurrentHookPoint->Use();
	bIsPulling = true;

	// Set rope end offset to target pull actor
	PullOffset = PullActorRef->GetOwner()->GetActorLocation() - GrapplePointPosition;
	PullSpeed = PullSpeed;

	// Trigger animation montage depending on initial state (grounded or mid air)
	if (GetCharacterMovement()->IsFalling())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(AirPullMontage);
	}
	else
	{
		GetMesh()->GetAnimInstance()->Montage_Play(GroundPullMontage);
	}
}

// Handles target and rope pulling movement towards the player
void AAgileCharacter::PullingMovement(float DeltaTime)
{
	// If it's not moving using grapple return
	if (!bMovingWithPull)
		return;

	// Set offseted direction from the hand to the hook mesh
	FVector Direction = GetMesh()->GetSocketLocation(TEXT("hand_l")) - 
		Hook->GetComponentLocation() +
		FollowCamera->GetForwardVector() * PullForwardOffset;
	Direction.Normalize();

	// Add force to the pulled object
	PullActorRef->AddForceAtLocation(Direction * PullSpeed * DeltaTime, CurrentHookPoint->GetActorLocation());

	// Set hook and rope visuals
	Hook->SetWorldLocation(CurrentHookPoint->GetActorLocation());

	RopeBaseLength = FVector::Distance(Hook->GetComponentLocation(), GetActorLocation());
	Rope->EndLocation = Hook->GetComponentLocation();

	// If the pulled object is close enough to the character, stop pulling
	if (FVector::Distance(GetMesh()->GetSocketLocation(TEXT("hand_l")), Hook->GetComponentLocation()) < PullMinDistance)
	{
		ResetPullMovement();
		return;
	}
}

// Reset pulling movement, return to normal state
void AAgileCharacter::ResetPullMovement()
{
	if (!bIsPulling || !PullActorRef)
		return;

	// If it was pulling a pull actor
	if (bMovingWithPull)
	{
		// Check if the pull actor is standing still or close to it
		// If so stop pulling and cancel throw
		FVector CurrentVelocity = PullActorRef->GetPhysicsLinearVelocity();
		if (CurrentVelocity.Size() <= ThrowCurrentVelocitySizeThreshold)
		{
			EndPull();
			return;
		}

		FVector ThrowForceVector;

		// Calculate throw force considering the direction of the player's camera and given offset
		FVector Offset = (GetActorLocation() - PullActorRef->GetOwner()->GetActorLocation());
		Offset.Normalize();
		ThrowForceVector = FollowCamera->GetForwardVector() * 3.0f + Offset;

		// Check the dotproduct of the current velocity of the pull actor and the calculated throw force
		// If it's less than the given threshold, the camera hasn't moved enough
		// Stop pulling and cancel throw
		ThrowForceVector.Normalize();
		CurrentVelocity.Normalize();
		if (CurrentVelocity != FVector::ZeroVector && FVector::DotProduct(CurrentVelocity, ThrowForceVector) < ThrowDotProductThreshold)
		{
			EndPull();
			return;
		}

		// Check for a valid throw target
		SetThrowTarget();

		// If one is fround recalculate throw force to launch pull actor againts throw target
		if (ThrowTarget != nullptr)
		{
			ThrowForceVector = (ThrowTarget->GetActorLocation() +
				FVector::UpVector * FVector::Distance(ThrowTarget->GetActorLocation(), PullActorRef->GetOwner()->GetActorLocation()) * ThrowOffset)
				- PullActorRef->GetOwner()->GetActorLocation();
			ThrowForceVector += FollowCamera->GetForwardVector();
		}
		// else add the current velocity times the given effect percentage to the calculated force, so it's more natural
		else
		{
			ThrowForceVector += CurrentVelocity * ThrowCurrentVelocityEffect;
		}
		ThrowForceVector.Normalize();

		// Reset pull actor's current velocity
		PullActorRef->SetPhysicsLinearVelocity(FVector::ZeroVector);
		PullActorRef->SetPhysicsAngularVelocity(FVector::ZeroVector);

		// Add throw force to pull actor
		PullActorRef->AddImpulse(ThrowForceVector * ThrowForce, NAME_None, true);
	}
	
	// Stop pulling
	EndPull();
}

// Handles movement of rope until it reaches the pull target
void AAgileCharacter::MovePullRope(float DeltaTime)
{
	if (!bIsPulling || bMovingWithPull)
		return;

	// Get current grapple montage position
	UAnimInstance* AnimeInstanceRef = GetMesh()->GetAnimInstance();
	float MontagePosition = AnimeInstanceRef->Montage_GetPosition(AnimeInstanceRef->GetCurrentActiveMontage());

	// Set initial target rope lenght as base value
	float TargetLength = RopeBaseLength;
	// Alpha value of end rope position lerp
	float TargetPosition;
	// Rope end position
	FVector RopeEndPosition;

	// Multiply rope length by correspondent montage float curve length
	// Set position lerp alpha to correspondent montage float curve length
	if (AnimeInstanceRef->GetCurrentActiveMontage() == GroundGrappleMontage)
	{
		TargetLength *= GroundRopeLength->GetFloatValue(MontagePosition);
		TargetPosition = GroundRopePosition->GetFloatValue(MontagePosition);
	}
	else
	{
		TargetLength *= AirRopeLength->GetFloatValue(MontagePosition);
		TargetPosition = AirRopePosition->GetFloatValue(MontagePosition);
	}
	// Set rope length
	Rope->CableLength = TargetLength;

	// Lerp rope end position from hand to end position by target position alpha
	RopeEndPosition = FMath::Lerp(GetMesh()->GetSocketLocation(TEXT("hand_l")), GrapplePointPosition, TargetPosition);
	Hook->SetWorldLocation(RopeEndPosition);
	// Set rope cable end location to same position as rope end mesh (Shouldn't be necessary but bugs you know?)
	Rope->EndLocation = RopeEndPosition;

	if (FVector::Distance(GrapplePointPosition, Hook->GetComponentLocation()) < PullSpeed * DeltaTime)
	{
		// Tick start of pulling object
		bMovingWithPull = true;
	}
}

void AAgileCharacter::SetThrowTarget()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
	ObjectTypesArray.Add(ObjectTypeQuery8); // Third custom object type == ThrowTargets
	TArray<AActor*> ActorsToIgnore;
	TArray<FHitResult> Hits;

	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), GetActorLocation(), GetActorLocation(), 2000.0f, ObjectTypesArray, false, ActorsToIgnore, EDrawDebugTrace::None, Hits, false);

	float HightestDotProduct = MinThrowTargetDot;
	ThrowTarget = nullptr;

	for (int i = 0; i < Hits.Num(); i++)
	{
		float DotProd = FVector::DotProduct(FollowCamera->GetForwardVector(), 
			UKismetMathLibrary::GetDirectionUnitVector(FollowCamera->GetComponentLocation(), Hits[i].GetActor()->GetActorLocation()));

		if (DotProd > HightestDotProduct)
		{
			ThrowTarget = Hits[i].GetActor();
			HightestDotProduct = DotProd;
		}
	}
}

void AAgileCharacter::EndPull()
{
	PullActorRef = nullptr;
	ThrowTarget = nullptr;

	// Return to normal animation
	GetMesh()->GetAnimInstance()->StopAllMontages(1.0f);
	SetRopeVisibility(false);

	// Stop pulling
	bMovingWithPull = false;
	bIsPulling = false;
}

// Called upon grapple attack input
void AAgileCharacter::GrappleAttackAction()
{
	// Do nothing if it's possessing another player character or with the notebook open
	if (bPossessing || bIsNotebookVisible)
		return;

	// Do nothing if the player character is dead or there is no valid hookpoint
	if (HealthComponent->IsDead() || !CurrentHookPoint)
		return;

	// Do nothing if the ability is on cooldown
	if (CurrentGrappleAttackCooldown > 0.0f)
		return;

	// If is grappling or can't grapple return
	if (bIsGrappleAttacking || CurrentHookPoint->Type != EHookType::ENEMY)
		return;

	// Don nothing if it's the middle of an attack animation and can't continue combo
	if (bInAttackAnimation && !bCanCombo)
		return;

	// If is player is too far off the grapple point to grapple return
	if (FVector::Distance(GetActorLocation(), CurrentHookPoint->GetActorLocation()) > GrappleThrowDistance)
		return;

	// Queue start of grapple attack
	bQueuedGrappleAttack = true;
	QueueSpecialAttack();
	bEndingCombo = false;
}

// Sets grapple attack end position
void AAgileCharacter::SetGrappleAttackDestination()
{
	GrappleDestination = GrappleAttackTarget->GetActorLocation() + GrappleAttackOffset - GetActorForwardVector() * GrappleAttackForwardOffset;
}

// Ticks cooldown and triggers beggining grapple attack
void AAgileCharacter::BeginGrappleAttack(float DeltaSeconds)
{
	// Tick ability's cooldown
	if (CurrentGrappleAttackCooldown > 0.0f)
	{
		CurrentGrappleAttackCooldown -= DeltaSeconds;
		return;
	}

	// Do nothing if the player character is dead or there is no valid hookpoint
	if (HealthComponent->IsDead() || !CurrentHookPoint)
		return;

	// Do nothing if the attack isn't queued or the character is in the middle of an attack animation
	if (!bQueuedGrappleAttack || bInAttackAnimation)
		return;

	EndUpperBoddyMontage();
	StopDash();
	
	SetGrappleAttackDestination();
	GrapplePointPosition = GrappleDestination;

	// Sets grapple starting position
	StartingPosition = GetActorLocation();

	// Rotate player towards the grapple destination
	FRotator TargetRotation = FRotator::MakeFromEuler(FVector::UpVector * 
		UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), GrappleAttackTarget->GetActorLocation()).Yaw);
	SetActorRotation(TargetRotation);

	// Tick grapple point visual cueu
	CurrentHookPoint->Use();

	// Set rope base length to distance from the player to the grapple destination
	RopeBaseLength = FVector::Distance(GrappleDestination, GetActorLocation());

	// Trigger animation montage depending on initial state (grounded or mid air)
	if (GetCharacterMovement()->IsFalling())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(AirGrappleAttackMontage);
	}
	else
	{
		GetMesh()->GetAnimInstance()->Montage_Play(GroundGrappleAttackMontage);
	}

	// The attack has now begun and isn't queued
	bIsGrappleAttacking = true;
	bQueuedGrappleAttack = false;
}

// Begins moving character through GrappleAttackMovement()
void AAgileCharacter::StartGrapplingAttackMovement()
{
	// Stops player in place, and sets default mode to falling
	GetCharacterMovement()->GravityScale = 0.0f;
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);

	// Ticks GrapplingMovement
	bMovingWithGrappleAttack = true;
}

// Handles grapple attack character movement
void AAgileCharacter::GrappleAttackMovement()
{
	// If it's not moving using grapple return
	if (!bMovingWithGrappleAttack)
		return;

	SetGrappleAttackDestination();
	GrapplePointPosition = GrappleDestination;

	// Get montage position for curves
	UAnimInstance* AnimeInstanceRef = GetMesh()->GetAnimInstance();
	float MontagePosition = AnimeInstanceRef->Montage_GetPosition(AnimeInstanceRef->GetCurrentActiveMontage());
	// Target location of the grapple
	FVector TargetLocation;

	// Set target position to lerp between grapple starting position and destination, with correspondent montage speed curve as alpha
	// Adds height offset as correspondent montage curve
	if (AnimeInstanceRef->GetCurrentActiveMontage() == GroundGrappleAttackMontage)
	{
		TargetLocation = FMath::Lerp(StartingPosition, GrappleDestination, 
			GroundAttackSpeedCurve->GetFloatValue(MontagePosition * 3.0f));
		TargetLocation += FVector::UpVector * 
			GroundAttackHeightOffsetCurve->GetFloatValue(MontagePosition * 3.0f);
	}
	else
	{
		TargetLocation = FMath::Lerp(StartingPosition, GrappleDestination, 
			AirAttackSpeedCurve->GetFloatValue(MontagePosition * 3.0f));
		TargetLocation += FVector::UpVector 
			* AirAttackHeightOffsetCurve->GetFloatValue(MontagePosition * 3.0f);
	}
	// Set player position to target location
	SetActorLocation(TargetLocation);
}

// Handles grapple attack rope movement
void AAgileCharacter::MoveGrappleAttackRope()
{
	if (!bIsGrappleAttacking)
		return;

	// Get current grapple montage position
	UAnimInstance* AnimeInstanceRef = GetMesh()->GetAnimInstance();
	float MontagePosition = AnimeInstanceRef->Montage_GetPosition(AnimeInstanceRef->GetCurrentActiveMontage());

	// Set initial target rope lenght as base value
	float TargetLength = RopeBaseLength;
	// Alpha value of end rope position lerp
	float TargetPosition;
	// Rope end position
	FVector RopeEndPosition;

	// Multiply rope length by correspondent montage float curve length
	// Set position lerp alpha to correspondent montage float curve length
	if (AnimeInstanceRef->GetCurrentActiveMontage() == GroundGrappleAttackMontage)
	{
		TargetLength *= GroundAttackRopeLength->GetFloatValue(MontagePosition);
		TargetPosition = GroundAttackRopePosition->GetFloatValue(MontagePosition);
	}
	else
	{
		TargetLength *= AirAttackRopeLength->GetFloatValue(MontagePosition);
		TargetPosition = AirAttackRopePosition->GetFloatValue(MontagePosition);
	}
	// Set rope length
	Rope->CableLength = TargetLength;

	// Lerp rope end position from hand to end position by target position alpha
	RopeEndPosition = FMath::Lerp(GetMesh()->GetSocketLocation(TEXT("hand_l")), GrapplePointPosition, TargetPosition);
	Hook->SetWorldLocation(RopeEndPosition);
	// Set rope cable end location to same position as rope end mesh (Shouldn't be necessary but bugs you know?)
	Rope->EndLocation = RopeEndPosition;
}

// Enable melee box in animation
void AAgileCharacter::DealGrappleAttackDamage()
{
	// If there is no valid target, do nothing
	if (!GrappleAttackTarget)
		return;

	// Deal damage to GrappleAttackTarget
	if (Cast<AEnemy>(GrappleAttackTarget) != nullptr)
		Cast<AEnemy>(GrappleAttackTarget)->TakeDamage(GrappleDamage);

	// Increase attack string
	CurrentAttackString++;
}

// Stops and resets the grapple attack
void AAgileCharacter::ResetGrappleAttack()
{
	// If it was using the ability, enter cooldown
	if (bIsGrappleAttacking)
		CurrentGrappleAttackCooldown = GrappleAttackCooldown;

	// Reset grapple attack values
	bMovingWithGrappleAttack = false;
	GetCharacterMovement()->GravityScale = InitialGravityScale;
	bIsGrappleAttacking = false;
	bQueuedSpecialAttack = false;
	EndAttackAnimation();
	SetRopeVisibility(false);
}

// Sets rope and hook visibility
void AAgileCharacter::SetRopeVisibility(bool bVisible)
{
	Rope->SetVisibility(bVisible);
	Hook->SetVisibility(bVisible);
}

// Handle dash input
void AAgileCharacter::DashAction()
{
	// Do nothing if it's possessing another player character or with the notebook open
	if (bPossessing || bIsNotebookVisible)
		return;

	// Do nothing if the player character is dead
	if (HealthComponent->IsDead())
		return;

	// Do nothing if the ability is in cooldown
	if (CurrentDashCooldown > 0.0f)
		return;

	// if it's already dashing do nothing
	if (bIsDashing)
		return;

	EndUpperBoddyMontage();

	// Stop attacking if that's the case
	StopCombo();
	EndAttackAnimation();
	EndAttack();

	// Stop grappling if that's the case
	ResetGrappleMovement();
	ResetGrappleAttack();

	// Stop pulling if that's the case
	EndPull();

	// Reset dash duration
	CurrentDashDistance = DashDistance;
	// Set target movement values
	GetCharacterMovement()->MaxWalkSpeed = DashSpeed;
	GetCharacterMovement()->MaxAcceleration = DashAcceleration;
	GetCharacterMovement()->GravityScale = 0.0f;
	// Play dash montage
	GetMesh()->GetAnimInstance()->Montage_Play(DashAnimation);
	// Tick dash
	bIsDashing = true;

	// If no input was given, dash forward
	if (FVector::Distance(DashDirection, FVector::ZeroVector) < 0.1f)
		DashDirection.X = 1.0f;

	// Get 2D Forward and Right Camera Vectors
	FVector CameraForward = FollowCamera->GetForwardVector();
	CameraForward.Z = 0.0f; CameraForward.Normalize();
	FVector CameraRight = FollowCamera->GetRightVector();
	CameraRight.Z = 0.0f; CameraRight.Normalize();

	// Join Camera vectors and input direction
	DashDirection = DashDirection.X * CameraForward + DashDirection.Y * CameraRight;
	DashDirection.Normalize();
}

// Handles dash movement
void AAgileCharacter::DashMovement(float DeltaTime)
{
	// Tick ability's cooldown
	if (CurrentDashCooldown > 0.0f)
	{
		CurrentDashCooldown -= DeltaTime;
		return;
	}

	// If it's done dashing, stop
	if (CurrentDashDistance <= 0)
	{
		StopDash();
		return;
	}

	// Move character along dash direction at dash speed
	AddMovementInput(DashDirection);

	// Decrease dash distance
	CurrentDashDistance -= GetCharacterMovement()->GetMaxSpeed() * DeltaTime;
}

void AAgileCharacter::StopDash()
{
	if (!bIsDashing)
		return;

	// Reset movement values
	GetCharacterMovement()->MaxWalkSpeed = NormalMaxSpeed;
	GetCharacterMovement()->MaxAcceleration = NormalMaxAcc;
	GetCharacterMovement()->GravityScale = InitialGravityScale;

	// Change velocity to current times the dash end factor
	if (GetCharacterMovement()->IsFalling())
		GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity * EndAirDashVelocityFactor;
	else
		GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity * EndGroundDashVelocityFactor;

	
	// Return to normal animation blueprint
	GetMesh()->GetAnimInstance()->Montage_Stop(0.5f, DashAnimation);
	
	// Tick to stop dashing
	bIsDashing = false;

	// Set cooldown
	CurrentDashCooldown = DashCooldown;
}

// Recieve damage
void AAgileCharacter::TakeDamage(float Amount)
{
	Super::TakeDamage(Amount);

	StopDash();
	ResetGrappleMovement();
	ResetPullMovement();
	ResetGrappleAttack();

	SetRopeVisibility(false);
}
