// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "PlayerCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "SoundManager.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(Mesh);
}

void AProjectile::Launch(FVector _Target, AActor* _VisualsRef)
{
	Target = _Target;
	ProjectileVisuals = _VisualsRef;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnBeginOverlap);

	UE_LOG(LogTemp, Warning, TEXT("Spawned %s."), *GetName());
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move(DeltaTime);
}

void AProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("Player"))
	{
		Cast<APlayerCharacter>(OtherActor)->TakeDamage(Damage);
		Explode();
	}
}

void AProjectile::Explode()
{
	if (ExplosionVfx != nullptr)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionVfx, GetActorLocation(), GetActorRotation());
	}

	SoundManager::PlayRandomSoundAtLocation(GetWorld(), ImpactSfx, GetActorLocation());

	if (ProjectileVisuals != nullptr)
		ProjectileVisuals->Destroy();

	Destroy();
}

void AProjectile::Move(float DeltaTime)
{
	if (FVector::Distance(GetActorLocation(), Target) < Speed * DeltaTime)
	{
		Explode();
	}

	SetActorLocation(GetActorLocation() + (Target - GetActorLocation()).GetSafeNormal() * Speed * DeltaTime);
}

