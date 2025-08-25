// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
#include "MP_Shooter/Character/PlayerCharacter.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if (!HasAuthority())
	{
		return;
	}

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	
	if (MuzzleFlashSocket)
	{
		FTransform MuzzleTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - MuzzleTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		playerCharacter = Cast<APlayerCharacter>(GetOwner());
		if (playerCharacter && !playerCharacter->IsAiming())
		{
			TargetRotation += AddRandomRotation();
		}

		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			UWorld* world = GetWorld();
			if (world)
			{
				world->SpawnActor<AProjectile>(
					ProjectileClass,
					MuzzleTransform.GetLocation(),
					TargetRotation,
					SpawnParams);
			}
		}
	}
}


FRotator AProjectileWeapon::AddRandomRotation()
{
	FRotator RandRotation = FRotator::ZeroRotator;

	RandRotation.Pitch = FMath::RandRange(-MaxRange, MaxRange);
	RandRotation.Yaw = FMath::RandRange(-MaxRange, MaxRange);
	RandRotation.Roll = FMath::RandRange(-MaxRange, MaxRange);
	
	return RandRotation;
}
