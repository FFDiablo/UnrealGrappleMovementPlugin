#include "GrappleCharacter.h"
// Include the original grapple component for backwards compatibility (no longer used)
#include "GrappleComponent.h"
#include "GrappleCharacterMovementComponent.h"
#include "EnhancedInputComponent.h"

AGrappleCharacter::AGrappleCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UGrappleCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    // Create the grapple component (deprecated).  Grapple logic has moved into
    // the character movement component, but we still instantiate the component
    // to avoid breaking existing blueprints.
    GrappleComp = CreateDefaultSubobject<UGrappleComponent>(TEXT("GrappleComponent"));
}

void AGrappleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Bind grapple controls on the movement component instead of the grapple component
        if (UGrappleCharacterMovementComponent* MoveComp = Cast<UGrappleCharacterMovementComponent>(GetCharacterMovement()))
        {
            MoveComp->SetupGrappleInput(EnhancedInput);
        }
    }
}