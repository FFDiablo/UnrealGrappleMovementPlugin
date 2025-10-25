// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "WA_PlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class WA_PLAYERMOVEMENTS_API AWA_PlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly) float CrouchBlendDuration = .2f;
	float CrouchBlendTime;

public:
	AWA_PlayerCameraManager();

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
};
