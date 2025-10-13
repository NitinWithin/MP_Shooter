// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameMode.h"
#include "MP_Shooter/Character/PlayerCharacter.h"
#include "MP_Shooter/PlayerController/PlayerCharacterController.h"
#include <Kismet/GameplayStatics.h>
#include <GameFramework/PlayerStart.h>

void AShooterGameMode::PlayerElim(APlayerCharacter* ElimChar, APlayerCharacterController* VictimController, APlayerController* AttkController)
{
	if (ElimChar)
	{
		ElimChar->PlayerDeath();
	}
}

void AShooterGameMode::RequestRespawn(ACharacter* ElimChar, AController* ElimController)
{
	if (ElimChar)
	{
		UE_LOG(LogTemp, Error, TEXT("Reset %s"), *ElimChar->GetName());
		ElimChar->Reset();
		ElimChar->Destroy();
	}
	if (ElimController)
	{
		TArray<AActor*> StartLocations;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), StartLocations);
		int32 RandSelection = FMath::RandRange(0, StartLocations.Num() - 1);
		RestartPlayerAtPlayerStart(ElimController, StartLocations[RandSelection]);
	}
}
