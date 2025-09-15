// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MP_SHOOTER_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PlayerElim(class APlayerCharacter* ElimChar, class APlayerCharacterController* AttkController);
};
