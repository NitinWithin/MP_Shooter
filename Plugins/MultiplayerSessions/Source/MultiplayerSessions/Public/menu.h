// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "menu.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API Umenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = "/Game/ThirdPerson/Maps/Lobby");

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

/// <summary>
/// Callback functions for the custom delegates on the multiplayersessionsubsystem class
/// </summary>
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SessionResults);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result, FString Address);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonOnClick();

	UFUNCTION()
	void JoinButtonOnClick();

	void RemoveMenu();

	 //The Subsystem designed to handle all online sessions functionality
	class UMultiplayerSessionSubsystem* MultiplayerSessionSubsystem;

	int32 NumOfConnections{ 4 };
	FString MatchType{ TEXT("FreeforAll") };
	FString PathToLobby;

};
