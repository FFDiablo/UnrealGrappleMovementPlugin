// All right reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WA_ByteArrayConverter.generated.h"

/**
 * 
 */
UCLASS()
class WA_PLAYERMOVEMENTS_API UWA_ByteArrayConverter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public: 
	UFUNCTION(BlueprintCallable, Category = "ByteArrayConverter")
	void ByteArrayToString(const TArray<uint8>& ByteArray, FString& OutString);

	UFUNCTION(BlueprintCallable, Category = "ByteArrayConverter")
	void StringToByteArray(const FString& InString, TArray<uint8>& OutByteArray);
	
};
