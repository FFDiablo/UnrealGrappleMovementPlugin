#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Texture2D.h"
#include "UWA_ItemData.generated.h"

UCLASS(Blueprintable)
class WA_PLAYERMOVEMENTS_API UWA_ItemData : public UObject
{
    GENERATED_BODY()

public:
    /** Item display name */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText ItemName;

    /** Optional item description */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText Description;

    /** Icon for UI */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    UTexture2D* Icon;

    /** How much space the item takes in inventory (grid size) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 SizeX = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 SizeY = 1;

    /** Can this item stack? */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stacking")
    bool bStackable = false;

    /** Max amount per stack (only relevant if bStackable is true) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stacking", meta = (EditCondition = "bStackable"))
    int32 MaxStack = 1;

    /** Base weight of a single unit */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    float Weight = 1.0f;

    /** Class to spawn in the world when dropped */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "World")
    TSubclassOf<AActor> WorldPickupClass;

    /** Item unique identifier */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName ItemID;
};