// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class PROJECTM_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* Trigger;

	virtual void Launch(FVector Target, AActor* VisualsRef = nullptr);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	UFUNCTION()
		virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere)
		float Speed = 100.0f;

	UPROPERTY(EditAnywhere)
		float Damage = 30.0f;

	UPROPERTY(EditAnywhere)
		float DestroyTimer = 3.0f;

	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* ExplosionVfx;

	void Explode();

	AActor* ProjectileVisuals;

	void Move(float DeltaTime);

	FVector Target;
};
