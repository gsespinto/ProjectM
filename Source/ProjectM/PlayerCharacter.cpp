// Copyright Epic Games, Inc. All Rights Reserved.

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
#include "HealthComponent.h"
#include "InteractionInterface.h"
#include "VenariGameInstance.h"
#include "ItemActor.h"
#include "DrawDebugHelpers.h"
#include "Enemy.h"
#include "Sound/SoundConcurrency.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "SoundManager.h"

//////////////////////////////////////////////////////////////////////////
// APlayerCharacter

APlayerCharacter::APlayerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a health component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));


	// Create Weapon Mesh
	Weapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon Mesh"));
	Weapon->SetupAttachment(GetMesh(), TEXT("hand_r"));

	// Create melee trigger box
	MeleeTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Melee Trigger"));
	MeleeTrigger->SetupAttachment(Weapon);
	MeleeTrigger->bEditableWhenInherited = true;
	
	// Create melee trigger box
	InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Interaction Trigger"));
	InteractionTrigger->SetupAttachment(RootComponent);
	InteractionTrigger->bEditableWhenInherited = true;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Set initial gravity to character movement's value
	bPossessed = (GetController()->GetNetOwningPlayer() != nullptr);

	MeleeTrigger->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnMeleeBoxBeginOverlap);	

	InteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnInteractionBoxBeginOverlap);
	InteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnInterationBoxOverlapEnd);

	GameInstance = Cast<UVenariGameInstance>(GetGameInstance());

	SetNotebookVisibility(false);
	bIsNotebookVisible = false;
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PossessCamMovement(DeltaSeconds);
	PlacingItem();
	TickStopAttackStreak(DeltaSeconds);
}

//////////////////////////////////////////////////////////////////////////
// Input

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	DECLARE_DELEGATE_OneParam(FFloatDelegate, float);
	DECLARE_DELEGATE_OneParam(FBoolDelegate, bool);

	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &APlayerCharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);

	// MeleeAttackAction input
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &APlayerCharacter::MeleeAttackAction);
	
	// Interact input
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerCharacter::InteractAction);

	PlayerInputComponent->BindAction("UseItem", IE_Pressed, this, &APlayerCharacter::ItemAction);
	PlayerInputComponent->BindAction("UseItem", IE_Released, this, &APlayerCharacter::PlaceAction);

	PlayerInputComponent->BindAction<FBoolDelegate>("InventoryWindow", IE_Pressed, this, &APlayerCharacter::SetInventoryVisibility, true);
	PlayerInputComponent->BindAction<FBoolDelegate>("InventoryWindow", IE_Released, this, &APlayerCharacter::SetInventoryVisibility, false);

	PlayerInputComponent->BindAction("NotebookWindow", IE_Released, this, &APlayerCharacter::TickNotebookVisibility);
}

void APlayerCharacter::AddControllerYawInput(float Value)
{
	if (bPossessing || bIsNotebookVisible || bInventoryVisible)
		return;

	Super::AddControllerYawInput(Value);
}

void APlayerCharacter::TurnAtRate(float Rate)
{
	RightAnalogInput.X = Rate;
	if (bPossessing || bIsNotebookVisible || bInventoryVisible)
		return;

	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::AddControllerPitchInput(float Value)
{

	if (bPossessing || bIsNotebookVisible || bInventoryVisible)
		return;

	Super::AddControllerPitchInput(Value);
}

void APlayerCharacter::LookUpAtRate(float Rate)
{
	RightAnalogInput.Y = Rate;
	if (bPossessing || bIsNotebookVisible || bInventoryVisible)
		return;

	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::MoveForward(float Value)
{
	if (bPossessing || bIsNotebookVisible)
		return;

	if (HealthComponent->IsDead())
		return;

	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::Jump()
{
	if (bPossessing || bIsNotebookVisible)
		return;

	if (HealthComponent->IsDead())
		return;

	if (bInAttackAnimation)
		return;

	ACharacter::Jump();
}

void APlayerCharacter::StopJumping()
{
	ACharacter::StopJumping();
}

void APlayerCharacter::MoveRight(float Value)
{
	if (bPossessing || bIsNotebookVisible)
		return;

	if (HealthComponent->IsDead())
		return;

	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

// Handle how character recieves damage
void APlayerCharacter::TakeDamage(float Amount)
{
	if (HealthComponent->IsDead())
		return;

	EndAttackAnimation();
	EndAttack();

	GetMesh()->GetAnimInstance()->StopAllMontages(0.0f);

	HealthComponent->TakeDamage(Amount);
	
	if (!HealthComponent->IsDead())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(HitMontage);
		SoundManager::PlayRandomSoundAtLocation(GetWorld(), DamagedSfx, GetActorLocation());
	}
	else
	{
		SoundManager::PlayRandomSoundAtLocation(GetWorld(), DeathSfx, GetActorLocation());
		SetInventoryVisibility(false);
		SetNotebookVisibility(false);
	}
}

void APlayerCharacter::BeginUpperBoddyMontage()
{
	bUpperBodyMontage = true;
}

void APlayerCharacter::EndUpperBoddyMontage()
{
	bUpperBodyMontage = false;
}

// Melee attack input
void APlayerCharacter::MeleeAttackAction()
{
	if (bPossessing || bIsNotebookVisible)
		return;

	if (HealthComponent->IsDead())
		return;

	if (AttackMontages.Num() <= 0)
		return;

	if (bInAttackAnimation && !bCanCombo)
		return;

	if (bCanCombo)
		bContinueCombo = true;

	if (bQueuedSpecialAttack)
		return;

	// Can't perform while falling (can be changed later to special falling attack)
	if (GetCharacterMovement()->IsFalling())
		return;

	// If it's not playing the attack montage already, do so
	if (!bInAttackAnimation)
	{
		EndUpperBoddyMontage();
		bInAttackAnimation = true;
		PlayAttackMontage(0);
		bInCombo = true;
		CurrentAttackString = 0;
		// SetCanCombo(true);
	}
}

// Move on to next attack animation
void APlayerCharacter::ContinueCombo()
{
	bInCombo = true;

	CurrentCombo++; CurrentAnimation++;
	CurrentAnimation = CurrentAnimation % AttackMontages.Num();

	PlayAttackMontage(CurrentAnimation);
	// SetCanCombo(true);
	bContinueCombo = false;
	bEndingCombo = false;
	bInAttackAnimation = true;
}

// Check if should end attacks or continue combo, called at the end of an attack animation
void APlayerCharacter::EndAttackAnimation()
{
	// It's not in an attack animation anymore
	// and can't continue combo
	bInAttackAnimation = false;
	SetCanCombo(false);

	// If the combo is limited and is at the end of the animation array
	// Won't continue the combo
	if (bLimitedCombo && CurrentAnimation >= AttackMontages.Num() - 1)
	{
		bContinueCombo = false;
		CurrentAttackString = 0;
	}

	// If there's a special attack queued, return to trigger special attack
	if (bQueuedSpecialAttack)
		return;

	// Continue combo if input was given and conditions were met
	if (bContinueCombo)
	{
		ContinueCombo();
		return;
	}

	StopCombo();
}

// Set to trigger special attack after current melee combo attack ends
void APlayerCharacter::QueueSpecialAttack()
{
	bQueuedSpecialAttack = true;
	bContinueCombo = false;
	SetCanCombo(false);
}

// Enable and disable input to continue combos
void APlayerCharacter::SetCanCombo(bool _CanCombo)
{
	bCanCombo = _CanCombo;
}

// Enable melee box in animation
void APlayerCharacter::BeginAttack()
{
	MeleeTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// Disable melee box in animation
void APlayerCharacter::EndAttack()
{
	MeleeTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Handle actors overlapping attack trigger to deal damage
void APlayerCharacter::OnMeleeBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
		return;

	if (OtherActor->ActorHasTag(TEXT("Enemy")))
	{
		if (Cast<AEnemy>(OtherActor) == nullptr)
			return;
		
		Cast<AEnemy>(OtherActor)->TakeDamage(Damage);
		SoundManager::PlayRandomSoundAtLocation(GetWorld(), ImpactSfx, GetActorLocation());

		// Increase current attack streak
		// Reset timer to stop streak
		CurrentAttackString++;
		currentTimeToStopAttackStreak = timeToStopAttackStreak;
	}
}
// Play given animation from animation attack array
void APlayerCharacter::PlayAttackMontage(int Index)
{
	if (AttackMontages.Num() <= Index)
		return;

	GetMesh()->GetAnimInstance()->Montage_Play(AttackMontages[Index]);
}

// Stop given animation from animation attack array
void APlayerCharacter::StopAttackMontage(int Index, float StopTime)
{
	if (AttackMontages.Num() <= Index)
		return;

	GetMesh()->GetAnimInstance()->Montage_Stop(StopTime, AttackMontages[Index]);
}

// Stop current attack animation, reset values
void APlayerCharacter::StopCombo()
{
	StopAttackMontage(CurrentAnimation, MeleeAnimationStopSpeed);
	CurrentCombo = 0; CurrentAnimation = 0;
	bContinueCombo = false; bInCombo = false;
	SetCanCombo(false);
	EndAttack();
}

// Demish ending combo timer, after which it'll stop the combo
void APlayerCharacter::TickStopAttackStreak(float DeltaTime)
{
	if (CurrentAttackString <= 0 || bInCombo)
		return;
	
	if (currentTimeToStopAttackStreak > 0)
	{
		currentTimeToStopAttackStreak -= DeltaTime;
		return;
	}

	CurrentAttackString = 0;
}

void APlayerCharacter::OnInteractionCPP(APlayerCharacter* Player)
{
	if (bPossessed)
	{
		return;
	}

	Player->StartPossession(this);
}

void APlayerCharacter::StartPossession(APlayerCharacter* PossessionTarget)
{
	PossessTarget = PossessionTarget;
	bPossessing = true;
}

void APlayerCharacter::PossessCamMovement(float DeltaSeconds)
{
	if (!bPossessing || PossessTarget == nullptr)
		return;

	// Lerp camera position to possess target camera position
	if (FVector::Distance(FollowCamera->GetComponentLocation(), PossessTarget->GetFollowCamera()->GetComponentLocation()) > PossessLocThres)
	{
		FollowCamera->SetWorldLocation(FMath::Lerp(FollowCamera->GetComponentLocation(), PossessTarget->GetFollowCamera()->GetComponentLocation(), PossessLocSpeed * DeltaSeconds));
	}

	// Lerp camera rotation to possess target camera rotation
	if (FVector::Distance(FollowCamera->GetComponentRotation().Vector(), PossessTarget->GetFollowCamera()->GetComponentRotation().Vector()) > PossessRotThres)
	{
		FollowCamera->SetWorldRotation(FMath::Lerp(FollowCamera->GetComponentRotation(), PossessTarget->GetFollowCamera()->GetComponentRotation(), PossessRotSpeed * DeltaSeconds));
	}

	// If the camera position and rotation is within the threshold to the possess target camera
	// Possess target character
	// Reset the camera position and rotation
	if (FVector::Distance(FollowCamera->GetComponentLocation(), PossessTarget->GetFollowCamera()->GetComponentLocation()) < PossessLocThres &&
		FVector::Distance(FollowCamera->GetComponentRotation().Vector(), PossessTarget->GetFollowCamera()->GetComponentRotation().Vector()) < PossessRotThres)
	{
		GetController()->Possess(PossessTarget);
		bPossessed = false;
		bPossessing = false;

		PossessTarget->bPossessed = true;
		PossessTarget = nullptr;

		FollowCamera->ResetRelativeTransform();
		FollowCamera->ResetRelativeTransform();

		return;
	}
}

void APlayerCharacter::Play2DSound(USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier)
{
	UGameplayStatics::PlaySound2D(GetWorld(), Sound, VolumeMultiplier, PitchMultiplier);
}

// Checks if an new interactable enters the range
void APlayerCharacter::OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bPossessed || bPossessing)
		return;
	
	if (OtherActor == this)
		return;

	if (OtherActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
	{
		CheckCurrentInteractable(OtherActor);
		return;
	}

	TArray<UActorComponent*> Components;
	OtherActor->GetComponents(Components);
	for (int i = 0; i < Components.Num(); i++)
	{
		if (Components[i]->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
		{
			CheckCurrentInteractable(Components[i]);
			return;
		}
	}
}

// Checks if a detected interactable leaves range
void APlayerCharacter::OnInterationBoxOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!bPossessed || bPossessing)
		return;

	if (OtherActor == this)
		return;

	if (OtherActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
	{
		DisableInteractable(OtherActor);
		if (CurrentInteractable == OtherActor)
			CurrentInteractable = nullptr;
	}

	TArray<UActorComponent*> Components;
	OtherActor->GetComponents(Components);
	for (int i = 0; i < Components.Num(); i++)
	{
		if (Components[i]->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
		{
			DisableInteractable(Components[i]);
			if (CurrentInteractable == Components[i])
				CurrentInteractable = nullptr;
		}
	}	
}

void APlayerCharacter::CheckCurrentInteractable(UObject* Interactable)
{
	if (!CurrentInteractable)
	{
		CurrentInteractable = Interactable;
		EnableInteractable(CurrentInteractable);
		return;
	}

	AActor* NewInteractableActor = Cast<AActor>(Interactable);
	if (!NewInteractableActor)
		return;

	AActor* CurrentInteractableActor = Cast<AActor>(Interactable);

	if (!CurrentInteractableActor || FVector::Dist(NewInteractableActor->GetActorLocation(), GetActorLocation()) < FVector::Dist(CurrentInteractableActor->GetActorLocation(), GetActorLocation()))
	{
		DisableInteractable(CurrentInteractable);
		CurrentInteractable = Interactable;
		EnableInteractable(CurrentInteractable);
		return;
	}
}

// Interact with the interactable closer to the player
void APlayerCharacter::InteractAction()
{
	if (bPossessing || bIsNotebookVisible || bInAttackAnimation)
		return;

	EndUpperBoddyMontage();
	UseInteractable(CurrentInteractable);
}

void APlayerCharacter::UseInteractable(UObject* InteractableObj)
{
	// If the closest reference isn't valid run the function again
	if (InteractableObj == nullptr)
	{
		return;
	}

	// Execute closest interactable's action (BLUEPRINT)
	DisableInteractable(InteractableObj);
	IInteractionInterface::Execute_OnInteraction(InteractableObj, this);

	// Check if object was deleted during it's action call
	if (InteractableObj == nullptr)
		return;

	// Execute closest interactable's action (CPP)
	IInteractionInterface* CurrentInteractionInterface = Cast<IInteractionInterface>(InteractableObj);
	if (!CurrentInteractionInterface)
		return;

	CurrentInteractionInterface->OnDisableCPP();
	CurrentInteractionInterface->OnInteractionCPP(this);
}

void APlayerCharacter::EnableInteractable(UObject* InteractableObj)
{
	if (InteractableObj == nullptr)
	{
		return;
	}

	IInteractionInterface::Execute_OnEnable(InteractableObj);

	// Check if object was deleted during it's action call
	if (InteractableObj == nullptr)
		return;

	IInteractionInterface* CurrentInteractionInterface = Cast<IInteractionInterface>(InteractableObj);
	if (!CurrentInteractionInterface)
		return;

	CurrentInteractionInterface->OnEnableCPP();
}

void APlayerCharacter::DisableInteractable(UObject* InteractableObj)
{
	if (InteractableObj == nullptr)
	{
		return;
	}

	IInteractionInterface::Execute_OnDisable(InteractableObj);

	// Check if object was deleted during it's action call
	if (InteractableObj == nullptr)
		return;

	IInteractionInterface* CurrentInteractionInterface = Cast<IInteractionInterface>(InteractableObj);
	if (!CurrentInteractionInterface)
		return;

	CurrentInteractionInterface->OnDisableCPP();
}

// Uses or starts placing equipped item, depending on its type
void APlayerCharacter::ItemAction()
{
	if (bPossessing || bIsNotebookVisible || bInAttackAnimation)
		return;

	FItemStruct Item = GameInstance->InventoryData[GameInstance->EquippedItemIndex];

	// If no item is equipped, do nothing
	if (Item.Name.IsNone())
		return;

	// If no item actor is set, do nothing
	if (!Item.ItemActor)
		return;

	switch (Item.Type)
	{
		case EItemType::CONSUMABLE:
			Item.ItemActor.GetDefaultObject()->UseItem(this);
			break;

		case EItemType::PLACEABLE:
			// Line trace to check the place to spawn the item
			FHitResult LineHit;
			GetWorld()->LineTraceSingleByChannel(LineHit, FollowCamera->GetComponentLocation(),
				FollowCamera->GetComponentLocation() + FollowCamera->GetForwardVector() * PlacingDistance, ECC_Visibility);

			if (!LineHit.GetActor() || !LineHit.GetActor()->ActorHasTag("AllowedPlacing"))
				return;

			if (FVector::DotProduct(LineHit.ImpactNormal, FVector::UpVector) < PlacingDotProductThreshold)
				return;

			FVector Location = LineHit.ImpactPoint;
			FRotator Rotation = GetCapsuleComponent()->GetComponentRotation();
			FActorSpawnParameters SpawnParams;

			// Spawn item to start placing
			PlacingActorRef = GetWorld()->SpawnActor<AItemActor>(Item.ItemActor, Location, Rotation, SpawnParams);
			break;
	}
}

// Activates PlacingActorRef and finishes placement
void APlayerCharacter::PlaceAction()
{
	if (bPossessing || bIsNotebookVisible || bInAttackAnimation)
		return;

	// If it's not placing an item, do nothing
	if (!PlacingActorRef)
		return;

	PlacingActorRef->UseItem(this);
	PlacingActorRef = nullptr;
}

// Sets PlacingActorRef position and rotation through LineTracing
void APlayerCharacter::PlacingItem()
{
	// If it's not placing an item, do nothing
	if (!PlacingActorRef)
		return;

	FHitResult LineHit;
	GetWorld()->LineTraceSingleByChannel(LineHit, FollowCamera->GetComponentLocation(),
		FollowCamera->GetComponentLocation() + FollowCamera->GetForwardVector() * PlacingDistance, ECC_Visibility);

	if (!LineHit.GetActor() || !LineHit.GetActor()->ActorHasTag("AllowedPlacing"))
		return;

	if (FVector::DotProduct(LineHit.ImpactNormal, FVector::UpVector) < PlacingDotProductThreshold)
		return;

	PlacingActorRef->SetActorLocation(LineHit.ImpactPoint);

	FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(LineHit.ImpactPoint, LineHit.ImpactPoint + LineHit.ImpactNormal);
	PlacingActorRef->SetActorRotation(Rotation);
}

void APlayerCharacter::TickNotebookVisibility()
{
	bIsNotebookVisible = !bIsNotebookVisible;
	SetNotebookVisibility(bIsNotebookVisible);
}

