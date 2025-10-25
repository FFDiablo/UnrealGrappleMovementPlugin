// All right reserved.


#include "WA_BPC_Inventory.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UWA_BPC_Inventory::UWA_BPC_Inventory()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// ...
}


// Called when the game starts
void UWA_BPC_Inventory::BeginPlay()
{
    Super::BeginPlay();

    // Initialize grid
    if (MainInventoryGrid.Num() == 0)
    {
        MainInventoryGrid.SetNum(Main_GridSizeX * Main_GridSizeY);
    }

    if (BeltInventoryGrid.Num() == 0)
    {
        BeltInventoryGrid.SetNum(Belt_GridSizeX * Belt_GridSizeY);
	}

    if (StashInventoryGrid.Num() == 0)
    {
        StashInventoryGrid.SetNum(Stash_GridSizeX * Stash_GridSizeY);
    }
}

void UWA_BPC_Inventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UWA_BPC_Inventory, MainInventoryGrid);
    DOREPLIFETIME(UWA_BPC_Inventory, BeltInventoryGrid);
    DOREPLIFETIME(UWA_BPC_Inventory, StashInventoryGrid);
}


void UWA_BPC_Inventory::OnRep_MainInventoryGrid()
{
    OnMainInventoryChanged(); // Blueprint event
}

void UWA_BPC_Inventory::OnRep_BeltInventoryGrid()
{
    OnBeltInventoryChanged(); // Blueprint event
}

void UWA_BPC_Inventory::OnRep_StashInventoryGrid()
{
    OnStashInventoryChanged(); // Blueprint event
}

bool UWA_BPC_Inventory::CanItemFit(UWA_ItemData* Item, EInventoryType InventoryType, int32 X, int32 Y) const
{

    switch (InventoryType)
    {
    case EInventoryType::MAIN:
        return MainInventoryGrid.IsValidIndex(Y * Main_GridSizeX + X) && !MainInventoryGrid[Y * Main_GridSizeX + X].bOccupied;
        break;
    case EInventoryType::BELT:
        return BeltInventoryGrid.IsValidIndex(Y * Belt_GridSizeX + X) && !BeltInventoryGrid[Y * Belt_GridSizeX + X].bOccupied;
        break;
    case EInventoryType::STASH:
        return StashInventoryGrid.IsValidIndex(Y * Stash_GridSizeX + X) && !StashInventoryGrid[Y * Stash_GridSizeX + X].bOccupied;
        break;
    default:
        UE_LOG(LogTemp, Warning, TEXT("Unknown Inventory Type!"));
        return false;

    }
}

void UWA_BPC_Inventory::PlaceItem(UWA_ItemData* Item, EInventoryType InventoryType, int32 X, int32 Y)
{
    switch (InventoryType)
    {
    case EInventoryType::MAIN:
        if (MainInventoryGrid.IsValidIndex(Y * Main_GridSizeX + X))
        {
            MainInventoryGrid[Y * Main_GridSizeX + X].Item = Item;
            MainInventoryGrid[Y * Main_GridSizeX + X].X = X;
            MainInventoryGrid[Y * Main_GridSizeX + X].Y = Y;
            MainInventoryGrid[Y * Main_GridSizeX + X].bOccupied = true;
        }
        break;
    case EInventoryType::BELT:
        if (BeltInventoryGrid.IsValidIndex(Y * Belt_GridSizeX + X))
        {
            BeltInventoryGrid[Y * Belt_GridSizeX + X].Item = Item;
            BeltInventoryGrid[Y * Belt_GridSizeX + X].X = X;
            BeltInventoryGrid[Y * Belt_GridSizeX + X].Y = Y;
            BeltInventoryGrid[Y * Belt_GridSizeX + X].bOccupied = true;
        }
        break;
    case EInventoryType::STASH:
        if (StashInventoryGrid.IsValidIndex(Y * Stash_GridSizeX + X))
        {
            StashInventoryGrid[Y * Stash_GridSizeX + X].Item = Item;
            StashInventoryGrid[Y * Stash_GridSizeX + X].X = X;
            StashInventoryGrid[Y * Stash_GridSizeX + X].Y = Y;
            StashInventoryGrid[Y * Stash_GridSizeX + X].bOccupied = true;
        }
        break;
    default:
        UE_LOG(LogTemp, Warning, TEXT("Unknown Inventory Type!"));
        break;
    }
}





void UWA_BPC_Inventory::Server_AddItem_Implementation(UWA_ItemData * Item, EInventoryType InventoryToAdd, int32 X, int32 Y)
{
    switch (InventoryToAdd)
    {
        case EInventoryType::MAIN:
            UE_LOG(LogTemp, Log, TEXT("Inventory Type: MAIN"));
            if (CanItemFit(Item, InventoryToAdd, X, Y))
            {
                PlaceItem(Item, InventoryToAdd, X, Y);
                OnRep_MainInventoryGrid(); // Update immediately on server
            }
        break;

        case EInventoryType::BELT:
            UE_LOG(LogTemp, Log, TEXT("Inventory Type: BELT"));
            if (CanItemFit(Item, InventoryToAdd, X, Y))
            {
                PlaceItem(Item, InventoryToAdd, X, Y);
                OnRep_BeltInventoryGrid(); // Update immediately on server
            }
            break;

        case EInventoryType::STASH:
            UE_LOG(LogTemp, Log, TEXT("Inventory Type: STASH"));
            if (CanItemFit(Item, InventoryToAdd, X, Y))
            {
                PlaceItem(Item, InventoryToAdd, X, Y);
                OnRep_StashInventoryGrid(); // Update immediately on server
            }
            break;

        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown Inventory Type!"));
            break;
        }
}
