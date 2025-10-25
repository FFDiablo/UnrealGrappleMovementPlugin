// All right reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ChatMessageType.h"
#include "WA_BPC_Inventory.h"
#include "WA_PC_Game.generated.h"

/**
 * 
 */
UCLASS()
class WA_PLAYERMOVEMENTS_API AWA_PC_Game : public APlayerController
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WA Inventory") UWA_BPC_Inventory* WA_Inventory;
	
public:
	AWA_PC_Game();

	UFUNCTION(client, reliable)
	void ClientReceiveChatMessage(const EChatMessageType MessageType, const FString& PlayerName, const FString& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "WA PlayerController|Chat System")
	void OnMessageReceived(const EChatMessageType MessageType, const FString& PlayerName, const FString& Message);

	/*
	// Internal server-side function to send a single chunk
	UFUNCTION(Client, Reliable)
	void ClientReceiveChunk(AWA_PC_Game* PC_Player, int32 sexeChunkIndex, const TArray<uint8>& Chunk);

protected:
	
	UPROPERTY()
	TMap<int32, TArray<uint8>> ReceivedChunks;
	
	UPROPERTY()
	TArray<uint8> AssembledData;

	int32 LastReceivedChunkIndex = -1;

	
	// Called when the full data is reconstructed
	UFUNCTION(BlueprintImplementableEvent, Category = "DataStream")
	void OnReceiveFullData(const TArray<uint8>& FullData);
	*/
};