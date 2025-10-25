#include "GrappleCharacter.h"
#include "GrappleComponent.h"
#include "GrappleCharacterMovementComponent.h"
#include "EnhancedInputComponent.h"

AGrappleCharacter::AGrappleCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UGrappleCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    // Create the grapple component
    GrappleComp = CreateDefaultSubobject<UGrappleComponent>(TEXT("GrappleComponent"));
}

void AGrappleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (GrappleComp)
        {
            GrappleComp->SetupPlayerInputComponent(EnhancedInput);
        }
    }
}