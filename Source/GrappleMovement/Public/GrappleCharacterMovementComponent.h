#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "CableComponent.h"
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

    /** Maximum distance for a grapple in centimetres */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grapple")
    float MaxGrappleDistance;

    /** Minimum allowed rope length */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grapple")
    float MinRopeLength;

    /** Speed at which rope length changes when reeling in/out (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grapple")
    float ReelSpeed;

    /** Additional boost velocity applied when the player triggers a boost (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grapple")
    float BoostVelocity;

    /** Whether we are currently grappling (replicated) */
    UPROPERTY(ReplicatedUsing=OnRep_IsGrappling, BlueprintReadOnly, Category="Grapple")
    bool bIsGrappling;

    /** Anchor location in world space that the rope is attached to (replicated) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category="Grapple")
    FVector GrappleAnchor;

    /** Current rope length in centimetres (replicated) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category="Grapple")
    float CurrentRopeLength;

    /** Whether the player is currently holding the reel in input (replicated) */
    UPROPERTY(Replicated)
    bool bReelInHeld;

    /** Whether the player is currently holding the reel out input (replicated) */
    UPROPERTY(Replicated)
    bool bReelOutHeld;

    /** Set up input bindings for grappling.  Call this from the owning character's SetupPlayerInputComponent */
    UFUNCTION(BlueprintCallable, Category="Grapple")
    void SetupGrappleInput(class UEnhancedInputComponent* PlayerInputComponent);

    /** Input handlers */
    UFUNCTION(BlueprintCallable, Category="Grapple")
    void GrappleActionPressed();
    UFUNCTION(BlueprintCallable, Category="Grapple")
    void GrappleActionReleased();
    UFUNCTION(BlueprintCallable, Category="Grapple")
    void ReelInPressed();
    UFUNCTION(BlueprintCallable, Category="Grapple")
    void ReelInReleased();
    UFUNCTION(BlueprintCallable, Category="Grapple")
    void ReelOutPressed();
    UFUNCTION(BlueprintCallable, Category="Grapple")
    void ReelOutReleased();
    UFUNCTION(BlueprintCallable, Category="Grapple")
    void BoostPressed();

    /** Returns true if the character is currently grappling */
    UFUNCTION(BlueprintCallable, Category="Grapple")
    bool IsGrappling() const { return bIsGrappling; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    /** Applies pendulum constraints to velocity and position */
    void ApplyGrappleConstraints(float DeltaTime);

    /** Updates rope length based on reel input and replicates to clients */
    void UpdateRopeLength(float DeltaTime);

    /** Perform a line trace from the player's viewpoint to find a grapple anchor */
    bool PerformGrappleTrace(FVector& OutLocation) const;

    /** Start and stop grappling on the server */
    UFUNCTION(Server, Reliable)
    void Server_StartGrapple(const FVector& Start, const FVector& End);
    UFUNCTION(Server, Reliable)
    void Server_StopGrapple();
    UFUNCTION(Server, Reliable)
    void Server_SetReelIn(bool bReelIn);
    UFUNCTION(Server, Reliable)
    void Server_SetReelOut(bool bReelOut);
    UFUNCTION(Server, Reliable)
    void Server_Boost();

    /** RepNotify when bIsGrappling changes */
    UFUNCTION()
    void OnRep_IsGrappling();

    /** Create/destroy cable component for rope visualisation */
    void UpdateCable();

    /** Utility to get owning character */
    class ACharacter* GetCharacterOwner() const;

    /** Cable component for visualising rope (non replicated) */
    UPROPERTY(Transient)
    class UCableComponent* Cable;

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};