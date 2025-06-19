// Fill out your copyright notice in the Description page of Project Settings.


#include "menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionSubsystem.h"
#include "OnlineSessionSettings.h"


void Umenu::MenuSetup(int32 NumOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?Listen"), *LobbyPath);
	NumOfConnections = NumOfPublicConnections;
	MatchType = TypeOfMatch;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UWorld* World = GetWorld();

	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
	}

	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool Umenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonOnClick);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonOnClick);
	}
	return true;
}

void Umenu::NativeDestruct()
{
	Super::NativeDestruct();

	RemoveMenu();
}

void Umenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 10, FColor::Red, FString(TEXT("session created")));

		UWorld* world = GetWorld();
		if (world)
		{
			world->ServerTravel(PathToLobby);
		}
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 10, FColor::Red, FString(TEXT("session not created")));
		HostButton->SetIsEnabled(true);
	}
}

void Umenu::OnFindSessions(bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	if (MultiplayerSessionSubsystem == nullptr)
	{
		return;
	}
	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchType)
		{
			MultiplayerSessionSubsystem->JoinSession(Result);
			return;
		}
	}
	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void Umenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result, FString Address)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		APlayerController* playerContoller = GetGameInstance()->GetFirstLocalPlayerController();
		playerContoller->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
	else
	{
		JoinButton->SetIsEnabled(true);
	}
}

void Umenu::OnDestroySession(bool bWasSuccessful)
{
}

void Umenu::OnStartSession(bool bWasSuccessful)
{
	if (bWasSuccessful && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 10, FColor::Yellow, FString(TEXT("session Started")));

		//UWorld* world = GetWorld();
		//if (world)
		//{
		//	world->ServerTravel("/Game/ThirdPerson/Maps/Lobby?Listen");
		//}
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 10, FColor::Red, FString(TEXT("session not Started")));
	}
}

void Umenu::HostButtonOnClick()
{
	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->CreateSession(NumOfConnections, MatchType);
	}
}

void Umenu::JoinButtonOnClick()
{
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->FindSessions(10000);
	}
}

void Umenu::RemoveMenu()
{
	RemoveFromParent();
	UWorld* world = GetWorld();
	if (world)
	{
		APlayerController* playerController = world->GetFirstPlayerController();
		if (playerController)
		{
			FInputModeGameOnly InputModeData;
			playerController->SetInputMode(InputModeData);
			playerController->SetShowMouseCursor(false);
		}
	}
}
