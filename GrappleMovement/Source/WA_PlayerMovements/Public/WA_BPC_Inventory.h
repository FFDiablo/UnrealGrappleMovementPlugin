// All right reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "UWA_ItemData.h"
#include "WA_BPC_Inventory.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UWA_ItemData* Item = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 X = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Y = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentStack = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOccupied = false;
};

UENUM(BlueprintType)
enum EInventoryType
{
    MAIN  UMETA(DisplayName = "Main"),
    BELT  UMETA(DisplayName = "Belt"),
    STASH UMETA(DisplayName = "Stash")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WA_PLAYERMOVEMENTS_API UWA_BPC_Inventory : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWA_BPC_Inventory();

protected:
	virtual void BeginPlay() override;

public:
    /** Inventory Grid */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MainInventoryGrid, Category = "WA Inventory|Main Inventory|Variables")
    TArray<FInventorySlot> MainInventoryGrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WA Inventory|Main Inventory|Variables")
    int32 Main_GridSizeX = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WA Inventory|Main Inventory|Variables")
    int32 Main_GridSizeY = 14;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BeltInventoryGrid, Category = "WA Inventory|Belt Inventory|Variables")
    TArray<FInventorySlot> BeltInventoryGrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WA Inventory|Belt Inventory|Variables")
	int32 Belt_GridSizeX = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WA Inventory|Belt Inventory|Variables")
    int32 Belt_GridSizeY = 3;


    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StashInventoryGrid, Category = "WA Inventory|Stash Inventory|Variables")
    TArray<FInventorySlot> StashInventoryGrid;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WA Inventory|Stash Inventory|Variables")
	int32 Stash_GridSizeX = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WA Inventory|Stash Inventory|Variables")
	int32 Stash_GridSizeY = 17;


    /** Add item at grid location */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "WA Inventory|Functions")
    void Server_AddItem(UWA_ItemData* Item, EInventoryType InventoryToAdd, int32 X, int32 Y);

    /** Called when InventoryGrid is updated on clients */
    UFUNCTION()
    void OnRep_MainInventoryGrid();
    UFUNCTION()
    void OnRep_BeltInventoryGrid();
    UFUNCTION()
    void OnRep_StashInventoryGrid();

    /** Blueprint Event to update the UI */
    
    UFUNCTION(BlueprintImplementableEvent, Category = "WA Inventory|Implementable")
    void OnMainInventoryChanged();
    UFUNCTION(BlueprintImplementableEvent, Category = "WA Inventory|Implementable")
    void OnBeltInventoryChanged();
    UFUNCTION(BlueprintImplementableEvent, Category = "WA Inventory|Implementable")
    void OnStashInventoryChanged();

    /** Helper functions */
    bool CanItemFit(UWA_ItemData* Item, EInventoryType InventoryType, int32 X, int32 Y) const;
    void PlaceItem(UWA_ItemData* Item, EInventoryType InventoryType, int32 X, int32 Y);
};