// All right reserved.

#pragma once

#include "CoreMinimal.h"
#include "WA_PC_Game.h"
#include "ChatMessageType.h"
#include "GameFramework/GameMode.h"
#include "WA_Gamemode.generated.h"

UCLASS()
class WA_PLAYERMOVEMENTS_API AWA_Gamemode : public AGameMode
{
	GENERATED_BODY()

public:
	// Exposed to Blueprint: start streaming a struct to a player
	UFUNCTION(BlueprintCallable, Category = "WA Gamemode|DataStream")
	void StreamStructToClient(AWA_PC_Game* TargetPlayer, const TArray<uint8>& StructData);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "WA Gamemode|Chat System")
	void SERVER_SendChatMessage(const EChatMessageType MessageType,const FString& PlayerName, const FString & Message);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "WA Gamemode|Chat System")
	void MULTICAST_SendChatMessage(const EChatMessageType MessageType, const FString& PlayerName, const FString& Message);
};
