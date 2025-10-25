// Copyright Epic Games, Inc. All Rights Reserved.

#include "WA_PlayerMovementsBPLibrary.h"
#include "WA_PlayerMovements.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/NetConnection.h"

#define GET_CONNECTION	UNetConnection* PlayerNetConnection = UWA_PlayerMovementsBPLibrary::Internal_GetNetConnection(WorldContextObject)

bool UWA_PlayerMovementsBPLibrary::GetMaxPacket(const UObject* WorldContextObject, int32& OutMaxPacket)
{
	if (GET_CONNECTION)
	{
		OutMaxPacket = PlayerNetConnection->MaxPacket;
		return true;
	}

	OutMaxPacket = INDEX_NONE;
	return false;
}

ENetDebugStatConnectionState UWA_PlayerMovementsBPLibrary::GetConnectionState(const UObject* WorldContextObject)
{
	ENetDebugStatConnectionState CurrentState = ENetDebugStatConnectionState::Invalid;

	if (GET_CONNECTION)
	{
		const EConnectionState& ConnectionState = PlayerNetConnection->GetConnectionState();
		switch (ConnectionState)
		{
		case USOCK_Closed:
			CurrentState = ENetDebugStatConnectionState::Closed;
			break;
		case USOCK_Pending:
			CurrentState = ENetDebugStatConnectionState::Pending;
			break;
		case USOCK_Open:
			CurrentState = ENetDebugStatConnectionState::Open;
			break;
		default:
			break;
		}


	}

	return CurrentState;
}

bool UWA_PlayerMovementsBPLibrary::GetAverageLag(const UObject* WorldContextObject, float& OutAverageLag)
{
	if (GET_CONNECTION)
	{
		OutAverageLag = PlayerNetConnection->AvgLag;
		return true;
	}

	OutAverageLag = -1.f;
	return false;
}

bool UWA_PlayerMovementsBPLibrary::GetBytes(const UObject* WorldContextObject, FNetDebugStatBytes& OutBytes, const bool bConvertToKB /*= true*/)
{
	if (GET_CONNECTION)
	{
		int32 InByte = PlayerNetConnection->InBytes;
		int32 OutByte = PlayerNetConnection->OutBytes;

		OutBytes = FNetDebugStatBytes(bConvertToKB ? InByte / 1000.f : InByte, bConvertToKB ? OutByte / 1000.f : OutByte);
		return true;
	}

	OutBytes = FNetDebugStatBytes();
	return false;
}

bool UWA_PlayerMovementsBPLibrary::GetBytesPerSecond(const UObject* WorldContextObject, FNetDebugStatBytes& OutBytes, const bool bConvertToKB /*= true*/)
{
	if (GET_CONNECTION)
	{
		int32 InByte = PlayerNetConnection->InBytesPerSecond;
		int32 OutByte = PlayerNetConnection->OutBytesPerSecond;

		OutBytes = FNetDebugStatBytes(bConvertToKB ? InByte / 1000.f : InByte, bConvertToKB ? OutByte / 1000.f : OutByte);
		return true;
	}

	OutBytes = FNetDebugStatBytes();
	return false;
}

bool UWA_PlayerMovementsBPLibrary::GetTotalBytes(const UObject* WorldContextObject, FNetDebugStatBytes& OutBytes, const bool bConvertToKB /*= true*/)
{
	if (GET_CONNECTION)
	{
		int32 InByte = PlayerNetConnection->InTotalBytes;
		int32 OutByte = PlayerNetConnection->OutTotalBytes;

		OutBytes = FNetDebugStatBytes(bConvertToKB ? InByte / 1000.f : InByte, bConvertToKB ? OutByte / 1000.f : OutByte);
		return true;
	}

	OutBytes = FNetDebugStatBytes();
	return false;
}

bool UWA_PlayerMovementsBPLibrary::GetPackets(const UObject* WorldContextObject, FNetDebugStatPackets& OutPackets)
{
	if (GET_CONNECTION)
	{
		OutPackets = FNetDebugStatPackets(PlayerNetConnection->InPackets, PlayerNetConnection->OutPackets);
		return true;
	}

	OutPackets = FNetDebugStatPackets();
	return false;
}

bool UWA_PlayerMovementsBPLibrary::GetPacketsPerSecond(const UObject* WorldContextObject, FNetDebugStatPackets& OutPackets)
{
	if (GET_CONNECTION)
	{
		OutPackets = FNetDebugStatPackets(PlayerNetConnection->InPacketsPerSecond, PlayerNetConnection->OutPacketsPerSecond);
		return true;
	}

	OutPackets = FNetDebugStatPackets();
	return false;
}

bool UWA_PlayerMovementsBPLibrary::GetTotalPackets(const UObject* WorldContextObject, FNetDebugStatPackets& OutPackets)
{
	if (GET_CONNECTION)
	{
		OutPackets = FNetDebugStatPackets(PlayerNetConnection->InTotalPackets, PlayerNetConnection->OutTotalPackets);
		return true;
	}

	OutPackets = FNetDebugStatPackets();
	return false;
}

bool UWA_PlayerMovementsBPLibrary::GetPacketLost(const UObject* WorldContextObject, FNetDebugStatPackets& OutPackets)
{
	if (GET_CONNECTION)
	{
		OutPackets = FNetDebugStatPackets(PlayerNetConnection->InPacketsLost, PlayerNetConnection->OutPacketsLost);
		return true;
	}

	OutPackets = FNetDebugStatPackets();
	return false;
}

bool UWA_PlayerMovementsBPLibrary::GetTotalAcks(const UObject* WorldContextObject, int32& OutTotalAcks)
{
	if (GET_CONNECTION)
	{
		OutTotalAcks = PlayerNetConnection->OutTotalAcks;
		return true;
	}

	OutTotalAcks = INDEX_NONE;
	return false;
}

bool UWA_PlayerMovementsBPLibrary::GetPacketLostPercentage(const UObject* WorldContextObject, float& InPercentage, float& OutPercentage, const bool bGetAverage /*= false*/)
{
	if (GET_CONNECTION)
	{
		if (bGetAverage)
		{
			InPercentage = PlayerNetConnection->GetInLossPercentage().GetAvgLossPercentage();
			OutPercentage = PlayerNetConnection->GetOutLossPercentage().GetAvgLossPercentage();
		}
		else
		{
			InPercentage = PlayerNetConnection->GetInLossPercentage().GetLossPercentage();
			OutPercentage = PlayerNetConnection->GetOutLossPercentage().GetLossPercentage();
		}
		return true;
	}

	InPercentage = OutPercentage = -1.f;
	return false;
}

class UNetConnection* UWA_PlayerMovementsBPLibrary::Internal_GetNetConnection(const UObject* WorldContextObject)
{
	const APlayerController* MyController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (MyController)
	{
		UNetConnection* MyNetConnection = MyController->GetNetConnection();
		return MyNetConnection;
	}

	return nullptr;
}

#undef GET_CONNECTION

