// All right reserved.


#include "WA_PC_Game.h"
#include "ChatMessageType.h"

AWA_PC_Game::AWA_PC_Game()
{
	//ReceivedChunks.Empty();
	//AssembledData.Empty();
	//LastReceivedChunkIndex = -1;

	WA_Inventory = CreateDefaultSubobject<UWA_BPC_Inventory>(TEXT("InventoryComponent"));

}

void AWA_PC_Game::ClientReceiveChatMessage_Implementation(const EChatMessageType MessageType, const FString& PlayerName, const FString& Message)
{
	// This function is called on the client to handle chat messages.
	// You can implement your logic here, such as displaying the message in the UI.
	UE_LOG(LogTemp, Log, TEXT("Chat Message from %s: %s"), *PlayerName, *Message);
	// Notify Blueprint or child class
	OnMessageReceived(MessageType, PlayerName, Message);
}


/*
void AWA_PC_Game::ClientReceiveChunk_Implementation(AWA_PC_Game* PC_Player, int32 sexeChunkIndex, const TArray<uint8>& Chunk)
{
	

	ReceivedChunks.Add(ChunkIndex, Chunk);
	LastReceivedChunkIndex = FMath::Max(LastReceivedChunkIndex, ChunkIndex);

	// Try to reassemble
	AssembledData.Empty();

	for (int32 i = 0; i <= LastReceivedChunkIndex; ++i)
	{
		if (!ReceivedChunks.Contains(i))
		{
			// Still missing a chunk
			return;
		}

		AssembledData.Append(ReceivedChunks[i]);
	}

	// All chunks received
	UE_LOG(LogTemp, Warning, TEXT("Full data received (%d bytes)."), AssembledData.Num());

	// Notify Blueprint or child class
	OnReceiveFullData(AssembledData);

	// Optional: clear buffers
	ReceivedChunks.Empty();
	LastReceivedChunkIndex = -1;

	
}

void AWA_PC_Game::OnReceiveFullData(const TArray<uint8>&FullData)
{
	// This function is intentionally left empty.
	// It can be overridden in derived classes if needed.	
}

*/