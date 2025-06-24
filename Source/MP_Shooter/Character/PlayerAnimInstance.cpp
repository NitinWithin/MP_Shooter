// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	playerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());	
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (playerCharacter == nullptr)
	{
		playerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}

	if (playerCharacter == nullptr)
	{
		return;
	}

	FVector Velocity = playerCharacter->GetVelocity();
	Velocity.Z = 0;
	speed = Velocity.Size();

	bIsInAir = playerCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = playerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bWeaponEquipped = playerCharacter->IsWeaponEquipped();
}
