// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoShell.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

AAmmoShell::AAmmoShell()
{
	PrimaryActorTick.bCanEverTick = false;

	AmmoCasing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ammo Casing"));
	SetRootComponent(AmmoCasing);

	AmmoCasing->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	AmmoCasing->SetSimulatePhysics(true);
	AmmoCasing->SetEnableGravity(true);
	AmmoCasing->SetNotifyRigidBodyCollision(true);

	AmmoEjectionImpulse = 10.f;
}


void AAmmoShell::BeginPlay()
{
	Super::BeginPlay();

	AmmoCasing->OnComponentHit.AddDynamic(this, &AAmmoShell::OnHit);
	AmmoCasing->AddImpulse(GetActorForwardVector() * AmmoEjectionImpulse); 
}

void AAmmoShell::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (AmmoImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AmmoImpactSound, GetActorLocation());
	}
	Destroy();
}

