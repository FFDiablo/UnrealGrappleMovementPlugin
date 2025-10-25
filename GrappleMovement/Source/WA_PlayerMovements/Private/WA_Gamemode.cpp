#include "WA_GameMode.h"
#include "GameFramework/PlayerController.h"

void AWA_Gamemode::StreamStructToClient(AWA_PC_Game* TargetPlayer, const TArray<uint8>& StructData)
{
	const int32 ChunkSize = 1024; // Example chunk size (adjust as needed)

	int32 TotalChunks = FMath::CeilToInt((float)StructData.Num() / ChunkSize);

	for (int32 i = 0; i < TotalChunks; ++i)
	{
		int32 StartIndex = i * ChunkSize;
		int32 BytesLeft = StructData.Num() - StartIndex;
		int32 Size = FMath::Min(ChunkSize, BytesLeft);

		TArray<uint8> Chunk;
		Chunk.Append(&StructData[StartIndex], Size);


		//TargetPlayer->ClientReceiveChunk(TargetPlayer, i, Chunk);
	}
}

void AWA_Gamemode::SERVER_SendChatMessage_Implementation(const EChatMessageType MessageType, const FString& PlayerName, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("WI SERVER | Chat Message from %s: %s"), *PlayerName, *Message);
	MULTICAST_SendChatMessage(MessageType, PlayerName, Message);
}

void AWA_Gamemode::MULTICAST_SendChatMessage_Implementation(const EChatMessageType MessageType, const FString& PlayerName, const FString& Message)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AWA_PC_Game* PlayerController = Cast<AWA_PC_Game>(*Iterator);
		if (PlayerController)
		{
			PlayerController->ClientReceiveChatMessage(MessageType, PlayerName, Message);
		}
	}
}
