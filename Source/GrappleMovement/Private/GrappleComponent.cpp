#include "GrappleComponent.h"
#include "GrappleCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "CableComponent.h"
#include "Net/UnrealNetwork.h"

UGrappleComponent::UGrappleComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);

    MaxGrappleDistance = 5000.f; // 50 metres in cm
    MinRopeLength = 200.f;
    ReelSpeed = 600.f;
    BoostVelocity = 1500.f;

    bIsGrappling = false;
    bReelInHeld = false;
    bReelOutHeld = false;
    CurrentRopeLength = 0.f;
    GrappleMovementComponent = nullptr;
    Cable = nullptr;
}

void UGrappleComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache movement component
    GrappleMovementComponent = Cast<UGrappleCharacterMovementComponent>(GetCharacterOwner() ? GetCharacterOwner()->GetCharacterMovement() : nullptr);
}

void UGrappleComponent::SetupPlayerInputComponent(UEnhancedInputComponent* PlayerInputComponent)
{
    if (!PlayerInputComponent)
    {
        return;
    }
    // The user is expected to set up input actions in their project and assign them here.
    // Bind dynamic actions for started/completed events.
    // Here we assume that the input actions have been created in Blueprints or by other means and are passed in via meta on the component.
    // For demonstration we provide functions that can be bound via project settings.
    PlayerInputComponent->BindActionByName(TEXT("Grapple"), ETriggerEvent::Started, this, &UGrappleComponent::GrappleActionPressed);
    PlayerInputComponent->BindActionByName(TEXT("Grapple"), ETriggerEvent::Completed, this, &UGrappleComponent::GrappleActionReleased);
    PlayerInputComponent->BindActionByName(TEXT("ReelIn"), ETriggerEvent::Started, this, &UGrappleComponent::ReelInPressed);
    PlayerInputComponent->BindActionByName(TEXT("ReelIn"), ETriggerEvent::Completed, this, &UGrappleComponent::ReelInReleased);
    PlayerInputComponent->BindActionByName(TEXT("ReelOut"), ETriggerEvent::Started, this, &UGrappleComponent::ReelOutPressed);
    PlayerInputComponent->BindActionByName(TEXT("ReelOut"), ETriggerEvent::Completed, this, &UGrappleComponent::ReelOutReleased);
    PlayerInputComponent->BindActionByName(TEXT("Boost"), ETriggerEvent::Started, this, &UGrappleComponent::BoostPressed);
}

void UGrappleComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner() || !GrappleMovementComponent)
    {
        return;
    }

    // Only update rope length and physics on authority (server) to maintain consistency
    if (GetOwner()->HasAuthority() && bIsGrappling)
    {
        UpdateRopeLength(DeltaTime);
    }

    // Even on clients, update cable visual to match replicated state
    UpdateCable();

    // Update movement component to reflect new rope state
    if (GrappleMovementComponent)
    {
        GrappleMovementComponent->bIsGrappling = bIsGrappling;
        GrappleMovementComponent->GrappleAnchor = GrappleAnchor;
        GrappleMovementComponent->GrappleRopeLength = CurrentRopeLength;
    }
}

void UGrappleComponent::GrappleActionPressed()
{
    // On local client send request to server to start grapple
    if (!bIsGrappling)
    {
        FVector HitLocation;
        if (PerformGrappleTrace(HitLocation))
        {
            // send to server
            Server_StartGrapple(GetCharacterOwner()->GetActorLocation(), HitLocation);
        }
    }
    else
    {
        // Already grappling; pressing again detaches
        Server_StopGrapple();
    }
}

void UGrappleComponent::GrappleActionReleased()
{
    // no behaviour on release
}

void UGrappleComponent::ReelInPressed()
{
    Server_SetReelIn(true);
}

void UGrappleComponent::ReelInReleased()
{
    Server_SetReelIn(false);
}

void UGrappleComponent::ReelOutPressed()
{
    Server_SetReelOut(true);
}

void UGrappleComponent::ReelOutReleased()
{
    Server_SetReelOut(false);
}

void UGrappleComponent::BoostPressed()
{
    Server_Boost();
}

void UGrappleComponent::Server_StartGrapple_Implementation(const FVector& Start, const FVector& End)
{
    if (bIsGrappling)
    {
        return;
    }
    ACharacter* CharOwner = GetCharacterOwner();
    if (!CharOwner || !GrappleMovementComponent)
    {
        return;
    }
    // compute distance
    float Distance = FVector::Dist(Start, End);
    if (Distance > MaxGrappleDistance)
    {
        return;
    }
    // set values
    BeginGrapple(End, Distance);
}

void UGrappleComponent::Server_StopGrapple_Implementation()
{
    EndGrapple();
}

void UGrappleComponent::Server_SetReelIn_Implementation(bool bReelIn)
{
    bReelInHeld = bReelIn;
    // ensure we don't reel in and out simultaneously
    if (bReelIn)
    {
        bReelOutHeld = false;
    }
}

void UGrappleComponent::Server_SetReelOut_Implementation(bool bReelOut)
{
    bReelOutHeld = bReelOut;
    if (bReelOut)
    {
        bReelInHeld = false;
    }
}

void UGrappleComponent::Server_Boost_Implementation()
{
    if (!bIsGrappling || !GrappleMovementComponent)
    {
        return;
    }
    ACharacter* CharOwner = GetCharacterOwner();
    if (!CharOwner)
    {
        return;
    }
    // direction toward anchor
    FVector Dir = (GrappleAnchor - CharOwner->GetActorLocation()).GetSafeNormal();
    GrappleMovementComponent->Velocity += Dir * BoostVelocity;
}

void UGrappleComponent::OnRep_IsGrappling()
{
    // When grapple state changes on a client, update cable component accordingly
    UpdateCable();
}

void UGrappleComponent::BeginGrapple(const FVector& Anchor, float Distance)
{
    bIsGrappling = true;
    GrappleAnchor = Anchor;
    CurrentRopeLength = FMath::Max(Distance, MinRopeLength);
    bReelInHeld = false;
    bReelOutHeld = false;
    OnRep_IsGrappling();
}

void UGrappleComponent::EndGrapple()
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

bool UGrappleComponent::PerformGrappleTrace(FVector& OutLocation)
{
    ACharacter* CharOwner = GetCharacterOwner();
    if (!CharOwner)
    {
        return false;
    }
    // Acquire a camera to trace from.  If the character has a controller with a PlayerCameraManager,
    // use it; otherwise fall back to actor rotation.
    FVector Start;
    FVector ForwardVector;
    if (AController* Controller = CharOwner->GetController())
    {
        FRotator ViewRot;
        Controller->GetPlayerViewPoint(Start, ViewRot);
        ForwardVector = ViewRot.Vector();
    }
    else
    {
        Start = CharOwner->GetActorLocation();
        ForwardVector = CharOwner->GetActorForwardVector();
    }
    FVector End = Start + ForwardVector * MaxGrappleDistance;
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

void UGrappleComponent::UpdateRopeLength(float DeltaTime)
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

void UGrappleComponent::UpdateCable()
{
    if (!GetCharacterOwner())
    {
        return;
    }
    if (bIsGrappling)
    {
        if (!Cable)
        {
            Cable = NewObject<UCableComponent>(GetOwner());
            if (Cable)
            {
                Cable->RegisterComponent();
                Cable->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
                Cable->SetAttachEndToComponent(nullptr, NAME_None);
                Cable->EndLocation = FVector::ZeroVector;
            }
        }
        if (Cable)
        {
            // start of cable at owner's root, end at anchor
            Cable->SetWorldLocation(GetOwner()->GetActorLocation());
            Cable->CableLength = CurrentRopeLength;
            // Compute the cable end location relative to the cable's component so that it reaches the anchor
            FVector LocalEnd = GetOwner()->GetActorTransform().InverseTransformPosition(GrappleAnchor);
            Cable->EndLocation = LocalEnd;
        }
    }
    else if (Cable)
    {
        Cable->DestroyComponent();
        Cable = nullptr;
    }
}

ACharacter* UGrappleComponent::GetCharacterOwner() const
{
    return Cast<ACharacter>(GetOwner());
}

void UGrappleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UGrappleComponent, bIsGrappling);
    DOREPLIFETIME(UGrappleComponent, GrappleAnchor);
    DOREPLIFETIME(UGrappleComponent, CurrentRopeLength);
    DOREPLIFETIME(UGrappleComponent, bReelInHeld);
    DOREPLIFETIME(UGrappleComponent, bReelOutHeld);
}