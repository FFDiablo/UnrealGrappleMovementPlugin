#include "GrappleCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CableComponent.h"
#include "Net/UnrealNetwork.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"

UGrappleCharacterMovementComponent::UGrappleCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Set reasonable defaults mirroring Worlds Adrift behaviour
    MaxGrappleDistance = 5000.f; // 50 metres in cm
    MinRopeLength = 200.f;
    ReelSpeed = 600.f;
    BoostVelocity = 1500.f;
    bIsGrappling = false;
    bReelInHeld = false;
    bReelOutHeld = false;
    CurrentRopeLength = 0.f;
    GrappleAnchor = FVector::ZeroVector;
    Cable = nullptr;
    SetIsReplicatedByDefault(true);
}

void UGrappleCharacterMovementComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UGrappleCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Only update rope length and handle physics on the server to maintain authority
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        if (bIsGrappling)
        {
            UpdateRopeLength(DeltaTime);
        }
    }

    // Apply grapple constraints on all machines to keep simulation consistent
    if (bIsGrappling)
    {
        ApplyGrappleConstraints(DeltaTime);
    }

    // Update cable visual (client & server)
    UpdateCable();
}

void UGrappleCharacterMovementComponent::ApplyGrappleConstraints(float DeltaTime)
{
    ACharacter* CharacterOwner = GetCharacterOwner();
    if (!CharacterOwner || CurrentRopeLength <= 0.f)
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
    if (Distance > CurrentRopeLength)
    {
        FVector TargetPos = GrappleAnchor + Direction * CurrentRopeLength;
        // Move actor directly; skip collision handling for this sample.  In a real game you
        // may want to sweep or test collision here.
        CharacterOwner->SetActorLocation(TargetPos, false);
        Distance = CurrentRopeLength;
    }

    // Remove radial velocity component pointing away from the anchor to simulate a pendulum
    FVector Vel = Velocity;
    float RadialVel = FVector::DotProduct(Vel, Direction);
    if (RadialVel > 0.f)
    {
        Vel -= Direction * RadialVel;
        Velocity = Vel;
    }
}

void UGrappleCharacterMovementComponent::SetupGrappleInput(UEnhancedInputComponent* PlayerInputComponent)
{
    if (!PlayerInputComponent)
    {
        return;
    }
    // Bind by action name; these names must exist in your project’s input system
    PlayerInputComponent->BindActionByName(TEXT("Grapple"), ETriggerEvent::Started, this, &UGrappleCharacterMovementComponent::GrappleActionPressed);
    PlayerInputComponent->BindActionByName(TEXT("Grapple"), ETriggerEvent::Completed, this, &UGrappleCharacterMovementComponent::GrappleActionReleased);
    PlayerInputComponent->BindActionByName(TEXT("ReelIn"), ETriggerEvent::Started, this, &UGrappleCharacterMovementComponent::ReelInPressed);
    PlayerInputComponent->BindActionByName(TEXT("ReelIn"), ETriggerEvent::Completed, this, &UGrappleCharacterMovementComponent::ReelInReleased);
    PlayerInputComponent->BindActionByName(TEXT("ReelOut"), ETriggerEvent::Started, this, &UGrappleCharacterMovementComponent::ReelOutPressed);
    PlayerInputComponent->BindActionByName(TEXT("ReelOut"), ETriggerEvent::Completed, this, &UGrappleCharacterMovementComponent::ReelOutReleased);
    PlayerInputComponent->BindActionByName(TEXT("Boost"), ETriggerEvent::Started, this, &UGrappleCharacterMovementComponent::BoostPressed);
}

void UGrappleCharacterMovementComponent::GrappleActionPressed()
{
    if (!bIsGrappling)
    {
        FVector HitLocation;
        if (PerformGrappleTrace(HitLocation))
        {
            // tell the server to start grappling
            Server_StartGrapple(GetCharacterOwner()->GetActorLocation(), HitLocation);
        }
    }
    else
    {
        Server_StopGrapple();
    }
}

void UGrappleCharacterMovementComponent::GrappleActionReleased()
{
    // no action on release
}

void UGrappleCharacterMovementComponent::ReelInPressed()
{
    Server_SetReelIn(true);
}

void UGrappleCharacterMovementComponent::ReelInReleased()
{
    Server_SetReelIn(false);
}

void UGrappleCharacterMovementComponent::ReelOutPressed()
{
    Server_SetReelOut(true);
}

void UGrappleCharacterMovementComponent::ReelOutReleased()
{
    Server_SetReelOut(false);
}

void UGrappleCharacterMovementComponent::BoostPressed()
{
    Server_Boost();
}

bool UGrappleCharacterMovementComponent::PerformGrappleTrace(FVector& OutLocation) const
{
    ACharacter* CharOwner = GetCharacterOwner();
    if (!CharOwner)
    {
        return false;
    }
    FVector Start;
    FVector Forward;
    if (AController* Controller = CharOwner->GetController())
    {
        FRotator ViewRot;
        Controller->GetPlayerViewPoint(Start, ViewRot);
        Forward = ViewRot.Vector();
    }
    else
    {
        Start = CharOwner->GetActorLocation();
        Forward = CharOwner->GetActorForwardVector();
    }
    FVector End = Start + Forward * MaxGrappleDistance;
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(CharOwner);
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
    if (bHit)
    {
        OutLocation = Hit.Location;
        return true;
    }
    return false;
}

void UGrappleCharacterMovementComponent::Server_StartGrapple_Implementation(const FVector& Start, const FVector& End)
{
    if (bIsGrappling)
    {
        return;
    }
    ACharacter* CharOwner = GetCharacterOwner();
    if (!CharOwner)
    {
        return;
    }
    float Distance = FVector::Dist(Start, End);
    if (Distance > MaxGrappleDistance)
    {
        return;
    }
    bIsGrappling = true;
    GrappleAnchor = End;
    CurrentRopeLength = FMath::Max(Distance, MinRopeLength);
    bReelInHeld = false;
    bReelOutHeld = false;
    OnRep_IsGrappling();
}

void UGrappleCharacterMovementComponent::Server_StopGrapple_Implementation()
{
    bIsGrappling = false;
    bReelInHeld = false;
    bReelOutHeld = false;
    CurrentRopeLength = 0.f;
    GrappleAnchor = FVector::ZeroVector;
    if (Cable)
    {
        Cable->DestroyComponent();
        Cable = nullptr;
    }
    OnRep_IsGrappling();
}

void UGrappleCharacterMovementComponent::Server_SetReelIn_Implementation(bool bReelIn)
{
    bReelInHeld = bReelIn;
    if (bReelIn)
    {
        bReelOutHeld = false;
    }
}

void UGrappleCharacterMovementComponent::Server_SetReelOut_Implementation(bool bReelOut)
{
    bReelOutHeld = bReelOut;
    if (bReelOut)
    {
        bReelInHeld = false;
    }
}

void UGrappleCharacterMovementComponent::Server_Boost_Implementation()
{
    if (!bIsGrappling)
    {
        return;
    }
    ACharacter* CharOwner = GetCharacterOwner();
    if (!CharOwner)
    {
        return;
    }
    FVector Dir = (GrappleAnchor - CharOwner->GetActorLocation()).GetSafeNormal();
    Velocity += Dir * BoostVelocity;
}

void UGrappleCharacterMovementComponent::OnRep_IsGrappling()
{
    UpdateCable();
}

void UGrappleCharacterMovementComponent::UpdateCable()
{
    ACharacter* CharOwner = GetCharacterOwner();
    if (!CharOwner)
    {
        return;
    }
    if (bIsGrappling)
    {
        if (!Cable)
        {
            Cable = NewObject<UCableComponent>(CharOwner);
            if (Cable)
            {
                Cable->RegisterComponent();
                Cable->AttachToComponent(CharOwner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
                Cable->SetAttachEndToComponent(nullptr, NAME_None);
            }
        }
        if (Cable)
        {
            Cable->SetWorldLocation(CharOwner->GetActorLocation());
            Cable->CableLength = CurrentRopeLength;
            FVector LocalEnd = CharOwner->GetActorTransform().InverseTransformPosition(GrappleAnchor);
            Cable->EndLocation = LocalEnd;
        }
    }
    else if (Cable)
    {
        Cable->DestroyComponent();
        Cable = nullptr;
    }
}

void UGrappleCharacterMovementComponent::UpdateRopeLength(float DeltaTime)
{
    if (!bIsGrappling)
    {
        return;
    }
    float DeltaLength = ReelSpeed * DeltaTime;
    if (bReelInHeld)
    {
        CurrentRopeLength = FMath::Max(CurrentRopeLength - DeltaLength, MinRopeLength);
    }
    else if (bReelOutHeld)
    {
        CurrentRopeLength = FMath::Min(CurrentRopeLength + DeltaLength, MaxGrappleDistance);
    }
}

ACharacter* UGrappleCharacterMovementComponent::GetCharacterOwner() const
{
    return Cast<ACharacter>(GetOwner());
}

void UGrappleCharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UGrappleCharacterMovementComponent, bIsGrappling);
    DOREPLIFETIME(UGrappleCharacterMovementComponent, GrappleAnchor);
    DOREPLIFETIME(UGrappleCharacterMovementComponent, CurrentRopeLength);
    DOREPLIFETIME(UGrappleCharacterMovementComponent, bReelInHeld);
    DOREPLIFETIME(UGrappleCharacterMovementComponent, bReelOutHeld);
}