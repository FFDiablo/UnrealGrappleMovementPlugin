// Copyright Epic Games, Inc. All Rights Reserved.

#include "WA_PlayerMovements.h"

#define LOCTEXT_NAMESPACE "FWA_PlayerMovementsModule"

void FWA_PlayerMovementsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FWA_PlayerMovementsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWA_PlayerMovementsModule, WA_PlayerMovements)