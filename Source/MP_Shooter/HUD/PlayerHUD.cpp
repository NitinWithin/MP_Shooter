// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"

void APlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		FVector2D ViewportCenter = FVector2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float CrosshairSpreadScaled = MaxCrosshairSpread * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter, Spread);
		}

		if (HUDPackage.CrosshairTop)
		{
			FVector2D Spread(0.f, -CrosshairSpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter, Spread);
		}

		if (HUDPackage.CrosshairRight)
		{
			FVector2D Spread(CrosshairSpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter, Spread);
		}

		if (HUDPackage.CrosshairLeft)
		{
			FVector2D Spread(-CrosshairSpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter, Spread);
		}

		if (HUDPackage.CrosshairBottom)
		{
			FVector2D Spread(0.f, CrosshairSpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter, Spread);
		}
	}
}

void APlayerHUD::DrawCrosshair(UTexture2D* Crosshair, FVector2D Position, FVector2D CrosshairSpread)
{
	const float TextureWidth = Crosshair->GetSizeX();
	const float TextureHeight = Crosshair->GetSizeY();

	const FVector2D TextureDrawPoint(
		Position.X - (TextureWidth / 2.f) + CrosshairSpread.X,
		Position.Y - (TextureHeight / 2.f) + CrosshairSpread.Y
	);

	DrawTexture(
		Crosshair,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		FLinearColor::White
	);
}
