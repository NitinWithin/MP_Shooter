// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "MP_Shooter/Weapon/Weapon.h"
#include "MP_Shooter/Character/PlayerCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
//#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MP_Shooter/PlayerController/PlayerCharacterController.h"
#include "MP_Shooter/HUD/PlayerHUD.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming)
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetHUDCrosshair(DeltaTime);
}

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if (playerCharacter == nullptr || playerCharacter->Controller == nullptr)
	{
		return;
	}

	playerController = playerController == nullptr ? Cast<APlayerCharacterController>(playerCharacter->Controller) : playerController;
	if (playerController)
	{
		playerHUD = playerHUD == nullptr ? Cast<APlayerHUD>(playerController->GetHUD()) : playerHUD;

		if (playerHUD)
		{
			FHUDPackage HUDPackage;
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
			}
			else
			{
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairTop = nullptr;
				HUDPackage.CrosshairBottom = nullptr;
				HUDPackage.CrosshairRight = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
			}
			//Calculate CrosshairSpread
			FVector2D WalkSpeedRange(0.f, playerCharacter->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D SpreadMultiplierRange(0.f, 1.f);
			FVector Velocity = playerCharacter->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, SpreadMultiplierRange, Velocity.Size());

			if (playerCharacter->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);	
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 25.f);	

			}
			HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairInAirFactor;
			playerHUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);

	if (playerCharacter && bIsAiming)
	{
		playerCharacter->GetCharacterMovement()->MaxWalkSpeed = playerCharacter->WalkingSpeed;
		playerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	}	
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (playerCharacter && bIsAiming)
	{
		playerCharacter->GetCharacterMovement()->MaxWalkSpeed = playerCharacter->WalkingSpeed;
		playerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && playerCharacter)
	{
		playerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		playerCharacter->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::Shoot(bool bPressed)
{
	bIsShooting = bPressed;

	if (bIsShooting)
	{
		FHitResult HitResult;
		TraceTheCrossHair(HitResult);
		ServerFire(HitResult.ImpactPoint);
	}

}

void UCombatComponent::TraceTheCrossHair(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrossHairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrossHairWorldPosition, CrossHairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrossHairLocation,
		CrossHairWorldPosition,
		CrossHairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrossHairWorldPosition;
		FVector End = Start + CrossHairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		); 
	}
}


void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}

	if (playerCharacter)
	{
		playerCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}



void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (playerCharacter == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	//Attach weapon to socket
	const USkeletalMeshSocket* HandSocket = playerCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, playerCharacter->GetMesh());
	}

	EquippedWeapon->SetOwner(playerCharacter);
}


