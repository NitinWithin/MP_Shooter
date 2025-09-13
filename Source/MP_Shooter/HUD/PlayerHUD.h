// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairLeft;

	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

UCLASS()
class MP_SHOOTER_API APlayerHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void DrawHUD() override;
private:
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
	float MaxCrosshairSpread = 16.f;

	void DrawCrosshair(UTexture2D* Crosshair, FVector2D Position, FVector2D CrosshairSpread, FLinearColor Color);

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; };
};
	