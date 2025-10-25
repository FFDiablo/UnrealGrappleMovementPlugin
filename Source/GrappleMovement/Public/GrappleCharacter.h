#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GrappleCharacter.generated.h"

    // Forward declaration for the old grapple component (kept for backwards compatibility)
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

        /**
         * Deprecated grapple component.  The grappling logic has been moved
         * into the custom character movement component, so this component is
         * no longer used.  It is kept here for backwards compatibility and
         * to avoid breaking existing blueprints.  It is not ticked or bound
         * to input.
         */
        UPROPERTY()
        UGrappleComponent* GrappleComp;
};