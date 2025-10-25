#include "GrappleCharacterMovementComponent.h"
#include "GameFramework/Character.h"

UGrappleCharacterMovementComponent::UGrappleCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsGrappling = false;
    GrappleRopeLength = 0.f;
    GrappleAnchor = FVector::ZeroVector;
}

void UGrappleCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsGrappling)
    {
        ApplyGrappleConstraints(DeltaTime);
    }
}

void UGrappleCharacterMovementComponent::ApplyGrappleConstraints(float DeltaTime)
{
    // Ensure owner exists
    ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
    if (!CharacterOwner || GrappleRopeLength <= 0.f)
    {
        return;
    }

    // Current position relative to anchor
    FVector Location = CharacterOwner->GetActorLocation();
    FVector ToAnchor = Location - GrappleAnchor;
    float Distance = ToAnchor.Size();
    if (Distance < KINDA_SMALL_NUMBER)
    {
        return;
    }
    FVector Direction = ToAnchor / Distance;

    // If we are beyond rope length, snap the character back to the end of the rope
    if (Distance > GrappleRopeLength)
    {
        FVector TargetPos = GrappleAnchor + Direction * GrappleRopeLength;
        // Move actor directly; skip collision handling for this sample.  In a real game you
        // may want to sweep or test collision here.
        CharacterOwner->SetActorLocation(TargetPos, false);
        Distance = GrappleRopeLength;
    }

    // Remove radial velocity component pointing away from the anchor to simulate a pendulum
    FVector Vel = Velocity;
    float RadialVel = FVector::DotProduct(Vel, Direction);
    // We only remove positive radial velocity (moving away from anchor); allow movement
    // toward anchor so reeling can shorten the rope
    if (RadialVel > 0.f)
    {
        Vel -= Direction * RadialVel;
        Velocity = Vel;
    }
}