// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "HealthComponent.h"
#include "Components/BoxComponent.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SoundManager.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create health component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// Create melee trigger box
	MeleeTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Melee Trigger"));
	MeleeTrigger->SetupAttachment(RootComponent);
	MeleeTrigger->bEditableWhenInherited = true;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	MeleeTrigger->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnMeleeBoxBeginOverlap);

	EndMeleeAttack();

	for (int i = 0; i < GetMesh()->GetMaterials().Num(); i++)
	{
		DynamicMaterials.Add(UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(i), this));
		GetMesh()->SetMaterial(i, DynamicMaterials[i]);
	}
}

void AEnemy::OnMeleeBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);

	if (Player == nullptr)
		return;

	Player->TakeDamage(MeleeDamage);
}

void AEnemy::TakeDamage(float Amount)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *GetName());

	if (HealthComponent->IsDead())
		return;

	HealthComponent->TakeDamage(Amount);

	if (DamageAnimation != nullptr)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(DamageAnimation);
	}

	for (int i = 0; i < DynamicMaterials.Num(); i++)
	{
		DynamicMaterials[i]->SetScalarParameterValue("HP Percentage", HealthComponent->GetHPRatio());
	}

	if (HealthComponent->IsDead())
	{
		if (DeathAnimations.Num() <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Missing death animations in %s."), *GetName());
		}
		else
		{
			int Index = FMath::RandRange(0, DeathAnimations.Num() - 1);
			GetMesh()->GetAnimInstance()->Montage_Play(DeathAnimations[Index]);
		}

		GetCapsuleComponent()->Deactivate();
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemy::UpdateWalkSpeed(float Value)
{
	GetCharacterMovement()->MaxWalkSpeed = Value;
}

void AEnemy::MeleeAttackAction()
{
	if (MeleeAttackAnimations.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Missing melee attack animations in %s."), *GetName());
		return;
	}

	int Index = FMath::RandRange(0, MeleeAttackAnimations.Num() - 1);
	GetMesh()->GetAnimInstance()->Montage_Play(MeleeAttackAnimations[Index]);
}

void AEnemy::BeginMeleeAttack()
{
	MeleeTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SoundManager::PlayRandomSoundAtLocation(GetWorld(), MeleeSfx, GetActorLocation());
}

void AEnemy::EndMeleeAttack()
{
	MeleeTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}