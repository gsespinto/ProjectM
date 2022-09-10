// Fill out your copyright notice in the Description page of Project Settings.


#include "BerserkerCharacter.h"
#include "PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "HealthComponent.h"
#include "CableComponent.h"
#include "Enemy.h"
#include "DestructableInterface.h"


ABerserkerCharacter::ABerserkerCharacter()
{
	// Create shoulder bash trigger box
	BashTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Shoulder Bash Trigger"));
	BashTrigger->SetupAttachment(RootComponent);
	BashTrigger->bEditableWhenInherited = true;
}

void ABerserkerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	DECLARE_DELEGATE_OneParam(FBoostDelegate, EBoostType);

	// Player Character input
	APlayerCharacter::SetupPlayerInputComponent(PlayerInputComponent);

	// Shoulder Bash
	PlayerInputComponent->BindAction("ShoulderBash", IE_Pressed, this, &ABerserkerCharacter::ShoulderBashAction);

	// Life Steal Boost
	PlayerInputComponent->BindAction<FBoostDelegate>("LifeStealBoost", IE_Pressed, this, &ABerserkerCharacter::UseBoost, EBoostType::LIFESTEAL);
	// Berserk Boost
	PlayerInputComponent->BindAction<FBoostDelegate>("BerserkBoost", IE_Pressed, this, &ABerserkerCharacter::UseBoost, EBoostType::BERSERK);
}

void ABerserkerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Assign overlap event to shoulder bash trigger box
	BashTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABerserkerCharacter::OnShoulderBashBeginOverlap);

	// Get normal initial speeds
	NormalSpeed = GetCharacterMovement()->MaxWalkSpeed;
	NormalAcceleration = GetCharacterMovement()->MaxAcceleration;
}

void ABerserkerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Shoulder Bash
	BeginShoulderBash();
	BashMovement(DeltaSeconds);
	TickBashCooldown(DeltaSeconds);
	
	// Boosts
	TickLifeSteal(DeltaSeconds);
	TickBerserkBoost(DeltaSeconds);
}

void ABerserkerCharacter::OnShoulderBashBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
		return;
	
	// If an enemy enters the trigger box
	// Damage it
	// And knockback
	if (OtherActor->ActorHasTag(TEXT("Enemy")))
	{
		FVector KnockDirection = OtherActor->GetActorLocation() - GetActorLocation();
		KnockDirection.Normalize();

		if (Cast<AEnemy>(OtherActor) != nullptr)
		{
			AEnemy* E = Cast<AEnemy>(OtherActor);
			E->TakeDamage(BashDamage);
	
			if (CurrentBashDistance <= BashDistance * (1 - BashDistancePercentToKnockback))
			{
				E->Knockback(KnockDirection * BashKnockbackForce);
			}
		}
		
		CurrentAttackString++;

		// If it's using the life steal boost
		// Heal percentage of given damage
		if (bUsingLifeSteal)
			HealthComponent->Heal(Damage * LifeStealPercent);
	}

	if (OtherActor->GetClass()->ImplementsInterface(UDestructableInterface::StaticClass()))
	{
		// Execute destructable's action (BLUEPRINT)
		IDestructableInterface::Execute_OnDestruction(OtherActor, this);

		// Check if object was deleted during it's action call
		if (OtherActor == nullptr)
			return;

		// Execute the destructable's action (CPP)
		IDestructableInterface* Destructable = Cast<IDestructableInterface>(OtherActor);
		if (!Destructable)
			return;

		Destructable->OnDestruction(this);
	}

	// Finish shoulder bash if it's supposed to
	if (bFinishBashUponImpact)
	{
		EndBashAttack();
		EndBashMovement();
	}
}

// Enable bash box in animation
void ABerserkerCharacter::BeginBashAttack()
{
	BashTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// Disable bash box in animation
void ABerserkerCharacter::EndBashAttack()
{
	BashTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void ABerserkerCharacter::MoveForward(float Value)
{
	// Can't move while bashing
	if (bIsBashing)
		return;

	APlayerCharacter::MoveForward(Value);
}

void ABerserkerCharacter::Jump()
{
	// Can't move while bashing
	if (bIsBashing)
		return;

	APlayerCharacter::Jump();
}

void ABerserkerCharacter::MoveRight(float Value)
{
	// Can't move while bashing
	if (bIsBashing)
		return;

	APlayerCharacter::MoveRight(Value);
}

void ABerserkerCharacter::ShoulderBashAction()
{
	// Do nothing if it's possessing another player character or with the notebook open
	if (bPossessing || bIsNotebookVisible)
		return;

	// Can't use shoulder bash if it's falling
	// Or on cooldown
	if (bIsBashing ||
		GetCharacterMovement()->IsFalling() ||
		CurrentBashCooldown > 0.0f)
		return;

	if (bInAttackAnimation && !bCanCombo)
		return;

	// Queue start of shoulder bash
	bQueuedBash = true;
	QueueSpecialAttack();
	bEndingCombo = false;
}

void ABerserkerCharacter::BeginShoulderBash()
{
	// Do nothing if the ability isn't queued or the character is in the middle of an attack animation
	if (!bQueuedBash || bInAttackAnimation)
		return;

	EndUpperBoddyMontage();

	// Play movement montage
	GetMesh()->GetAnimInstance()->Montage_Play(BashMovementAnim);
	// Set distance and direction
	CurrentBashDistance = BashDistance;
	BashDirection = GetCapsuleComponent()->GetForwardVector();

	// The ability has now begun and isn't queued
	bIsBashing = true;
	bQueuedBash = false;
}

// Reset values and trigger cooldown
void ABerserkerCharacter::EndShoulderBash()
{
	bIsBashing = false;
	bMoveWithBash = false;
	bQueuedSpecialAttack = false;
	CurrentBashCooldown = BashCooldown;
	EndAttackAnimation();
}

// Start shoulder bash movement
void ABerserkerCharacter::BeginBashMovement()
{
	// Tick start of shoulder bash movement
	bMoveWithBash = true;
	// Set movement values
	GetCharacterMovement()->MaxWalkSpeed = BashSpeed;
	GetCharacterMovement()->MaxAcceleration = BashAcceleration;
}

// End shoulder bash movement
void ABerserkerCharacter::EndBashMovement()
{
	bMoveWithBash = false;

	// Reset movement values
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	GetCharacterMovement()->MaxAcceleration = NormalAcceleration;
	GetCharacterMovement()->StopMovementImmediately();

	// Stop Bash Movement Montage
	APlayerCharacter::EndUpperBoddyMontage();
	// Play Bash End Montage
	GetMesh()->GetAnimInstance()->Montage_Play(BashEndAnim);
}

void ABerserkerCharacter::BashMovement(float DeltaSeconds)
{
	if (!bMoveWithBash)
		return;

	// Move character along dash direction at dash speed
	AddMovementInput(BashDirection);
	// Decrease dash distance
	CurrentBashDistance -= GetCharacterMovement()->GetMaxSpeed() * DeltaSeconds;

	// If it has done the bash distance, 
	// end bash attack and movement
	if (CurrentBashDistance <= 0.0f)
	{
		EndBashAttack();
		EndBashMovement();
	}
}

// Decrease bash cooldown
void ABerserkerCharacter::TickBashCooldown(float DeltaSeconds)
{
	if (CurrentBashCooldown < 0.0f)
		return;

	CurrentBashCooldown -= DeltaSeconds;
}

void ABerserkerCharacter::UseBoost(EBoostType BoostType)
{
	// Do nothing if the ability isn't queued or the character is in the middle of an attack animation
	if (bPossessing || bIsNotebookVisible)
		return;

	// Do nothing if dead
	if (HealthComponent->IsDead())
		return;

	// Check the type of boost
	// If it's using the boost or on cooldown, do nothing
	// Set boost duration and tick its usage
	switch (BoostType)
	{
		case EBoostType::LIFESTEAL:
			if (bUsingLifeSteal || CurrentLifeStealCooldown > 0.0f)
				return;
			CurrentLifeStealDuration = LifeStealDuration;
			bUsingLifeSteal = true;
			break;

		case EBoostType::BERSERK:
			if (bUsingBerserk || CurrentBerserkCooldown > 0.0f)
				return;
			OriginalDamage = Damage; // Store original value to reset damage later
			Damage = OriginalDamage + DamageBoost; // Add boost to original damage
			CurrentBerserkDuration = BerserkDuration;
			bUsingBerserk = true;
			break;
	}
}

void ABerserkerCharacter::TickLifeSteal(float DeltaSeconds)
{
	// Do nothing if dead
	if (HealthComponent->IsDead())
		return;

	// If it's not using the boost or on cooldown, do nothing
	if (!bUsingLifeSteal && CurrentLifeStealCooldown < 0.0f)
		return;

	// Tick boost duration
	if (CurrentLifeStealDuration > 0.0f)
	{
		CurrentLifeStealDuration -= DeltaSeconds;
		return;
	}
	// When it ends set boost cooldown and tick off its usage
	else if (bUsingLifeSteal)
	{
		CurrentLifeStealCooldown = LifeStealCooldown;
		bUsingLifeSteal = false;
		return;
	}

	// Tick cooldown
	CurrentLifeStealCooldown -= DeltaSeconds;
}

void ABerserkerCharacter::TickBerserkBoost(float DeltaSeconds)
{
	// Do nothing if dead
	if (HealthComponent->IsDead())
		return;

	// If it's not using the boost or on cooldown, do nothing
	if (!bUsingBerserk && CurrentBerserkCooldown < 0.0f)
		return;

	// Tick boost duration
	if (CurrentBerserkDuration > 0.0f)
	{
		CurrentBerserkDuration -= DeltaSeconds;
		return;
	}
	// When it ends set boost cooldown and tick off its usage
	else if (bUsingBerserk)
	{
		CurrentBerserkCooldown = BerserkCooldown;
		Damage = OriginalDamage;
		bUsingBerserk = false;
		return;
	}

	// Tick cooldown
	CurrentBerserkCooldown -= DeltaSeconds;
}

void ABerserkerCharacter::TakeDamage(float Amount)
{
	// If it's using the berserk boost
	// the player takes less damage
	if (bUsingBerserk)
		Amount = Amount * DamageDamping;

	if (bIsBashing)
		EndShoulderBash();
	
	Super::TakeDamage(Amount);
}

void ABerserkerCharacter::OnMeleeBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Do nothing if dead
	if (HealthComponent->IsDead())
		return;

	// If it's using the life steal boost
	// Heal percentage of given damage
	if (bUsingLifeSteal)
		HealthComponent->Heal(Damage * LifeStealPercent);

	Super::OnMeleeBoxBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void ABerserkerCharacter::ItemAction()
{
	if (bIsBashing)
		return;

	Super::ItemAction();
}

void ABerserkerCharacter::PlaceAction()
{
	if (bIsBashing)
		return;

	Super::PlaceAction();
}