// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MP_Shooter/HUD/PlayerHUD.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 8000000

class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MP_SHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class APlayerCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void Shoot(bool bPressed);

	void FireWeapon();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceTheCrossHair(FHitResult& TraceHitResult);

	void SetHUDCrosshair(float DeltaTime);

private:
	class APlayerCharacter* playerCharacter;
	class APlayerCharacterController* playerController;
	class APlayerHUD* playerHUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* equippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	bool bIsShooting;

	/*
		HUD and Crosshair	
	*/

	float crosshairVelocityFactor;
	float crosshairInAirFactor;
	float crosshairAimFactor;
	float crosshairShootingFactor;

	FVector hitTarget;
	FVector noHitTarget;

	/* Aiming and Fov */
	float defaultFOV;
	float currentFOV;
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float zoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float zoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);
	
	/*Auto Fire weapons*/
	FTimerHandle weaponFireTimerHandle;

	bool bCanFire;

	void FireTimerStart();
	void FireTimerEnd();

public:	
	void EquipWeapon(AWeapon* WeaponToEquip);
		
};
