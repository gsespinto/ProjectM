// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "HealthComponent.h"
#include "Components/BoxComponent.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	HealthComponent->TakeDamage(Amount);

	if (DamageAnimation != nullptr)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(DamageAnimation);
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
	if (MeleeAttackAnimation == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Missing melee attack animation reference in %s."), *GetName());
		return;
	}

	GetMesh()->GetAnimInstance()->Montage_Play(MeleeAttackAnimation);
}

void AEnemy::BeginMeleeAttack()
{
	MeleeTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::EndMeleeAttack()
{
	MeleeTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
