// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MP_Shooter/MP_ShooterTypes/TurningInPlace.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MP_SHOOTER_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private: 
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true))
	class APlayerCharacter* playerCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterMovement", meta = (AllowPrivateAccess = true))
	float speed;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterMovement", meta = (AllowPrivateAccess = true))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterMovement", meta = (AllowPrivateAccess = true))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterCombat", meta = (AllowPrivateAccess = true))
	bool bWeaponEquipped;

	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterMovement", meta = (AllowPrivateAccess = true))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterMovement", meta = (AllowPrivateAccess = true))
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterCombat", meta = (AllowPrivateAccess = true))
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterMovement", meta = (AllowPrivateAccess = true))
	float YawOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = "CharacterMovement", meta = (AllowPrivateAccess = true))
	float Lean;
	
	UPROPERTY(BlueprintReadOnly, Category = "CharacterCombat", meta = (AllowPrivateAccess = true))
	float AO_Yaw;
	
	UPROPERTY(BlueprintReadOnly, Category = "CharacterCombat", meta = (AllowPrivateAccess = true))
	float AO_Pitch;
	
	UPROPERTY(BlueprintReadOnly, Category = "CharacterCombat", meta = (AllowPrivateAccess = true))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterCombat", meta = (AllowPrivateAccess = true))
	FRotator RightHandRotation;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterMovement", meta = (AllowPrivateAccess = true))
	ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true))
	bool bLocallyControlled;
};
