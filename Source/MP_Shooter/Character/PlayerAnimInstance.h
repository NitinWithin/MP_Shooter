// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true))
	float speed;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true))
	bool bWeaponEquipped;
};
