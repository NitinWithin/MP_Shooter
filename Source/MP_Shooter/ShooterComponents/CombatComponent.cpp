// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "MP_Shooter/Weapon/Weapon.h"
#include "MP_Shooter/Character/PlayerCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MP_Shooter/PlayerController/PlayerCharacterController.h"
#include "Camera/CameraComponent.h"


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
	if (playerCharacter && playerCharacter->GetFollowCamera())
	{
		DefaultFOV = playerCharacter->GetFollowCamera()->FieldOfView; 
		CurrentFOV = DefaultFOV;
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (playerCharacter && playerCharacter->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceTheCrossHair(HitResult);
		if (HitResult.bBlockingHit)
		{
			
			HitTarget = HitResult.ImpactPoint;
		}
		else
		{
			HitTarget = NoHitTarget;
		}

		SetHUDCrosshair(DeltaTime);
		InterpFOV(DeltaTime);
	}
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
			float DefaultWeaponSpread = 0.f;
			if (EquippedWeapon)
			{
				DefaultWeaponSpread = EquippedWeapon->GetDefaultWeaponSpread();

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
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.82f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
			
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread = 
				DefaultWeaponSpread + 
				CrosshairVelocityFactor + 
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;
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
		ServerFire(HitTarget);

		if (EquippedWeapon)
		{
			CrosshairShootingFactor = EquippedWeapon->GetShootingSpread();
		}
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

	FVector Start = CrossHairWorldPosition;
	if (playerCharacter)
	{
		float DistanceToPlayer = (playerCharacter->GetActorLocation() - Start).Size();
		Start += CrossHairWorldDirection * (DistanceToPlayer + 50.f);
	}
	FVector End = Start + CrossHairWorldDirection * TRACE_LENGTH;

	if (bScreenToWorld)
	{
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		); 

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UCrosshairInteractionInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
	// If LineTrace doesnt hit anything, we set it to the END and set it to HITTARGET in the Tick
	NoHitTarget = End;
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

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(
			CurrentFOV,
			EquippedWeapon->GetZoomFOV(),
			DeltaTime,
			EquippedWeapon->GetZoomInterpSpeed()
		);
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(
			CurrentFOV,
			DefaultFOV,
			DeltaTime,
			EquippedWeapon->GetZoomInterpSpeed()
		);
	}

	if (playerCharacter && playerCharacter->GetFollowCamera())
	{
		playerCharacter->GetFollowCamera()->SetFieldOfView(CurrentFOV);
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
