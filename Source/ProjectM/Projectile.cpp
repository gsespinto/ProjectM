// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "PlayerCharacter.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetSimulatePhysics(true);

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(Mesh);
}

void AProjectile::Launch(FVector Direction, AActor* VisualsRef)
{
	Mesh->AddForce(Direction.GetSafeNormal() * Force);
	ProjectileVisuals = VisualsRef;
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

	TickDestroyTimer(DeltaTime);
}

void AProjectile::TickDestroyTimer(float DeltaTime)
{
	if (DestroyTimer > 0.0f)
	{
		DestroyTimer -= DeltaTime;
		return;
	}

	Explode();
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

	if (ProjectileVisuals != nullptr)
		ProjectileVisuals->Destroy();

	Destroy();
}

