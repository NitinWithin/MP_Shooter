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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	friend class APlayerCharacter;

protected:
	virtual void BeginPlay() override;

private:
	class APlayerCharacter* PlayerCharacter;
	AWeapon* EquippedWeapon;

public:	
	void EquipWeapon(AWeapon* WeaponToEquip);
		
};
