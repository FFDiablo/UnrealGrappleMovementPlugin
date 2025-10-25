// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Unreal Classes
class USpringArmComponent;
class UCameraComponent;

// WAMovements Classes
class AWACharacter;
class UWACharacterMovementComponent;
class AWACameraManager;

class FWA_PlayerMovementsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
