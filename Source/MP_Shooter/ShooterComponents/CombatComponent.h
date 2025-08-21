// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MP_SHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class APlayerCharacter;

	//Uncomment if needed
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void Shoot(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire();

private:
	class APlayerCharacter* playerCharacter;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	bool bIsShooting;

public:	
	void EquipWeapon(AWeapon* WeaponToEquip);
		
};
