#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"
#include "GrappleComponent.generated.h"

class UGrappleCharacterMovementComponent;

/**
 * UGrappleComponent handles line tracing, cable spawning, rope length management
 * and network replication for Worlds Adrift style grappling.  It relies on a
 * UGrappleCharacterMovementComponent on the owning character to apply
 * pendulum constraints to movement.  Inputs are bound via the Enhanced Input
 * system.  Grapple state (anchor point, rope length and whether the grapple
 * is active) is replicated to remote clients so that swinging is simulated on
 * all machines.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRAPPLEMOVEMENT_API UGrappleComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGrappleComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Bind input actions using the Enhanced Input system */
    UFUNCTION(BlueprintCallable, Category="Grapple")
    void SetupPlayerInputComponent(UEnhancedInputComponent* PlayerInputComponent);

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

protected:
    /** Whether we are currently grappling */
    UPROPERTY(ReplicatedUsing=OnRep_IsGrappling)
    bool bIsGrappling;

    /** Anchor location in world space that the rope is attached to */
    UPROPERTY(Replicated)
    FVector GrappleAnchor;

    /** Current rope length in centimetres */
    UPROPERTY(Replicated)
    float CurrentRopeLength;

    /** Whether the player is currently holding the reel in input */
    UPROPERTY(Replicated)
    bool bReelInHeld;

    /** Whether the player is currently holding the reel out input */
    UPROPERTY(Replicated)
    bool bReelOutHeld;

    // Cable component used for visualising the rope on owning client and remote clients
    UPROPERTY(Transient)
    class UCableComponent* Cable;

    /** Cached reference to owning character movement component */
    UGrappleCharacterMovementComponent* GrappleMovementComponent;

    /** Input functions */
    void GrappleActionPressed();
    void GrappleActionReleased();
    void ReelInPressed();
    void ReelInReleased();
    void ReelOutPressed();
    void ReelOutReleased();
    void BoostPressed();

    /** Server RPCs to control grapple state */
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

    /** Called when bIsGrappling changes */
    UFUNCTION()
    void OnRep_IsGrappling();

    void BeginGrapple(const FVector& Anchor, float Distance);
    void EndGrapple();

    /** Performs a line trace to find an anchor point based off the player's view */
    bool PerformGrappleTrace(FVector& OutLocation);

    /** Updates rope length based on reel input and replicates to clients */
    void UpdateRopeLength(float DeltaTime);

    /** Ensure cable component exists and attaches to the correct sockets */
    void UpdateCable();

    // Utility to get owner character
    class ACharacter* GetCharacterOwner() const;

public:
    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};