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
#include "TimerManager.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	bCanFire = true;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, equippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming)
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (playerCharacter && playerCharacter->GetFollowCamera())
	{
		defaultFOV = playerCharacter->GetFollowCamera()->FieldOfView; 
		currentFOV = defaultFOV;
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
			
			hitTarget = HitResult.ImpactPoint;
		}
		else
		{
			hitTarget = noHitTarget;
		}

		SetHUDCrosshair(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (playerCharacter == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}

	equippedWeapon = WeaponToEquip;
	equippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	//Attach weapon to socket
	const USkeletalMeshSocket* HandSocket = playerCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(equippedWeapon, playerCharacter->GetMesh());
	}

	equippedWeapon->SetOwner(playerCharacter);
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (equippedWeapon && playerCharacter)
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
		FireWeapon();
	}
}

void UCombatComponent::FireWeapon()
{
	if(bCanFire)
	{
		ServerFire(hitTarget);
		if (equippedWeapon)
		{
			bCanFire = false;
			crosshairShootingFactor = equippedWeapon->GetShootingSpread();
		}
		FireTimerStart();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (equippedWeapon == nullptr)
	{
		return;
	}

	if (playerCharacter)
	{
		playerCharacter->PlayFireMontage(bAiming);
		equippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::FireTimerStart()
{
	if (equippedWeapon == nullptr || playerCharacter == nullptr)
	{
		return;
	}
	playerCharacter->GetWorldTimerManager().SetTimer(
		weaponFireTimerHandle,
		this,
		&UCombatComponent::FireTimerEnd,
		equippedWeapon->GetFireRateDelay()
	);
}

void UCombatComponent::FireTimerEnd()
{
	bCanFire = true;
	if(bIsShooting && equippedWeapon->bAutomaticWeaponFire())
	{
		FireWeapon();
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (equippedWeapon == nullptr)
	{
		return;
	}

	if (bAiming)
	{
		currentFOV = FMath::FInterpTo(
			currentFOV,
			equippedWeapon->GetZoomFOV(),
			DeltaTime,
			equippedWeapon->GetZoomInterpSpeed()
		);
	}
	else
	{
		currentFOV = FMath::FInterpTo(
			currentFOV ,
			defaultFOV,
			DeltaTime,
			equippedWeapon->GetZoomInterpSpeed()
		);
	}

	if (playerCharacter && playerCharacter->GetFollowCamera())
	{
		playerCharacter->GetFollowCamera()->SetFieldOfView(currentFOV);
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
	noHitTarget = End;
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
			if (equippedWeapon)
			{
				DefaultWeaponSpread = equippedWeapon->GetDefaultWeaponSpread();

				HUDPackage.CrosshairCenter = equippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairTop = equippedWeapon->CrosshairTop;
				HUDPackage.CrosshairBottom = equippedWeapon->CrosshairBottom;
				HUDPackage.CrosshairRight = equippedWeapon->CrosshairRight;
				HUDPackage.CrosshairLeft = equippedWeapon->CrosshairLeft;
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

			crosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, SpreadMultiplierRange, Velocity.Size());

			if (playerCharacter->GetCharacterMovement()->IsFalling())
			{
				crosshairInAirFactor = FMath::FInterpTo(crosshairInAirFactor, 2.25f, DeltaTime, 2.25f);	
			}
			else
			{
				crosshairInAirFactor = FMath::FInterpTo(crosshairInAirFactor, 0.f, DeltaTime, 25.f);	
			}
			if (bAiming)
			{
				crosshairAimFactor = FMath::FInterpTo(crosshairAimFactor, 0.82f, DeltaTime, 30.f);
			}
			else
			{
				crosshairAimFactor = FMath::FInterpTo(crosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
			
			crosshairShootingFactor = FMath::FInterpTo(crosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread = 
				DefaultWeaponSpread + 
				crosshairVelocityFactor + 
				crosshairInAirFactor -
				crosshairAimFactor +
				crosshairShootingFactor;
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
		playerCharacter->GetCharacterMovement()->MaxWalkSpeed = playerCharacter->walkingSpeed;
		playerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	}	
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (playerCharacter && bIsAiming)
	{
		playerCharacter->GetCharacterMovement()->MaxWalkSpeed = playerCharacter->walkingSpeed;
		playerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}