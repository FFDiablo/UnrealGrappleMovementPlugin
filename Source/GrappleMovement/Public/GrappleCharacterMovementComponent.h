#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GrappleCharacterMovementComponent.generated.h"

/**
 * Custom character movement component that supports grappling‑style movement.
 * It removes radial velocity while the character is attached to a grapple, creating
 * a pendulum effect similar to the grapple mechanics in Worlds Adrift.  When the rope
 * length is exceeded the character is snapped back to the end of the rope to ensure
 * a constant length.  Rope length and anchor point are supplied by a separate
 * Grapple component.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRAPPLEMOVEMENT_API UGrappleCharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    UGrappleCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

    /** Anchor location replicated by the grapple component */
    UPROPERTY(BlueprintReadWrite, Category="Grapple")
    FVector GrappleAnchor;

    /** Current rope length set by the grapple component */
    UPROPERTY(BlueprintReadWrite, Category="Grapple")
    float GrappleRopeLength;

    /** Whether the character is currently attached to a grapple */
    UPROPERTY(BlueprintReadWrite, Category="Grapple")
    bool bIsGrappling;

protected:
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    /** Applies pendulum constraints to velocity and position */
    void ApplyGrappleConstraints(float DeltaTime);
};