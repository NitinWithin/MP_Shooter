
#include "PlayerCharacterController.h"
#include "MP_Shooter/HUD/PlayerHUD.h"
#include "MP_Shooter/HUD/PlayerOverlay.h"
#include <Components/ProgressBar.h>
#include <Components/TextBlock.h>


void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

	playerHUD = Cast<APlayerHUD>(GetHUD());
}

void APlayerCharacterController::SetPlayerHealthInHUD(float currentHealth, float maxHealth)
{
	playerHUD = playerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : playerHUD;

	if (playerHUD 
		&& playerHUD->playerOverlay
		&& playerHUD->playerOverlay->HealthBar
		&& playerHUD->playerOverlay->HealthText)
	{
		const float healthPercent = currentHealth / maxHealth;
		playerHUD->playerOverlay->HealthBar->SetPercent(healthPercent);
		FString healthtext = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(currentHealth), FMath::CeilToInt(maxHealth));
		playerHUD->playerOverlay->HealthText->SetText(FText::FromString(healthtext));
	}
}

