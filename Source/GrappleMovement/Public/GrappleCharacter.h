#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GrappleCharacter.generated.h"

class UGrappleComponent;
class UGrappleCharacterMovementComponent;

/**
 * A character class that integrates the GrappleComponent and uses the
 * GrappleCharacterMovementComponent to implement Worlds Adrift‑style grappling.
 */
UCLASS()
class GRAPPLEMOVEMENT_API AGrappleCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AGrappleCharacter(const FObjectInitializer& ObjectInitializer);

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    /** Grapple component used by this character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grapple")
    UGrappleComponent* GrappleComp;
};