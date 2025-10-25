// Fill out your copyright notice in the Description page of Project Settings.


#include "WA_PlayerCameraManager.h"

#include "WA_Character.h"
#include "GameFramework/Character.h"
#include "WA_CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AWA_PlayerCameraManager::AWA_PlayerCameraManager()
{
}

void AWA_PlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	if (AWA_Character* WACharacter = Cast<AWA_Character>(GetOwningPlayerController()->GetPawn()))
	{
		UWA_CharacterMovementComponent* WMC = WACharacter->GetWA_CharacterMovement();
		FVector TargetCrouchOffset = FVector(
			0,
			0,
			WMC->GetCrouchedHalfHeight() - WACharacter->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		);
		FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset, FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.f, 1.f));

		if (WMC->IsCrouching())
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.f, CrouchBlendDuration);
			Offset -= TargetCrouchOffset;
		}
		else
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.f, CrouchBlendDuration);
		}

		OutVT.POV.Location += Offset;
	}
}