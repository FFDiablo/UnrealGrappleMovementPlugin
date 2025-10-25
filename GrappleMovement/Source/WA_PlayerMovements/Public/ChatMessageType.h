#pragma once

#include "CoreMinimal.h"
#include "ChatMessageType.generated.h"

UENUM(BlueprintType)
enum EChatMessageType
{
    SERVER  UMETA(DisplayName = "Server"),
    CLIENT  UMETA(DisplayName = "Client"),
    LOG     UMETA(DisplayName = "Log")
};