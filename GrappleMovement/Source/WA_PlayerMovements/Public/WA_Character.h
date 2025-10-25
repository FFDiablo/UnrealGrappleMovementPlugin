// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WA_PlayerMovements.h"
#include "GameFramework/Character.h"
#include <Net/UnrealNetwork.h>
#include "WA_Character.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class WA_PLAYERMOVEMENTS_API AWA_Character : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement) class UWA_CharacterMovementComponent* WA_CharacterMovementComponent;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IA_Jump;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IA_Move;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IA_Look;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IA_MouseLook;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true")) USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true")) UCameraComponent* FollowCamera;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input) float TurnRateGamepad;

	/** MappingContext for player input. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnhancedInput")
	UInputMappingContext* InputMapping;

public:
	AWA_Character(const FObjectInitializer& ObjectInitializer);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple Properties")
	bool bUseOldInputs = false;
	
	UFUNCTION()
	void MoveAction(const FInputActionValue& Value);
	UFUNCTION()
	void LookAction(const FInputActionValue& Value);
	UFUNCTION()
	void TurnAtRateAction(const FInputActionValue& Value);
	UFUNCTION()
	void LookUpAtRateAction(const FInputActionValue& Value);

	void DoMove(float Right, float Forward);
	void DoLook(float Yaw, float Pitch);
	
	
	// Input
private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Grapple Inputs")
	void GrapplePressed();

	UFUNCTION(BlueprintCallable, Category = "Grapple Inputs")
	void GrappleReleased();

	UFUNCTION(BlueprintCallable, Category = "Grapple Inputs")
	void GrappleReel(float AxisValue);

	UFUNCTION(BlueprintCallable, Category = "Grapple Inputs")
	void ReleasedGrappleReel();

	UFUNCTION(BlueprintCallable, Category = "Grapple Inputs")
	void BoostGrapple();

	// APawn interface
protected:
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintPure) FORCEINLINE UWA_CharacterMovementComponent* GetWA_CharacterMovement() const { return WA_CharacterMovementComponent; }

	//Grapple
public: 

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsGrappling = false;

	UPROPERTY(BlueprintReadOnly, Replicated)
	FVector CurrentGrapplePoint = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadOnly, Replicated)
	float CurrentGrappleDistance = 0.0f;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float RopeLength = 0.0f;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool ReelIn = false;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool ReelOut = false;

	UPROPERTY(BlueprintReadOnly, Replicated)
	FVector HookLocationOffset;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Grapple Properties")
	bool UseNewGrapplePhys = false;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Grapple Properties")
	float SpringForce = 5000.0f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Grapple Properties")
	float SpringForceDamping = 1000.0f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Grapple Properties")
	float VelocityDamping = 0.3f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Grapple Properties")
	float MaxGrappleDistance = 5000.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Grapple Properties")
	float ClimbSpeed = 6.0f;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Grapple Properties")
	TSubclassOf<AAHookPoint> HookToSpawn;

	UPROPERTY(Replicated)
	AAHookPoint* Ref_HookPoint;


	void TrySetGrappleLocal();

	void SetGrapplePointLocal(FVector GrapplePoint, AActor* HitActor);

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Replicated)
	float Delta = 0.0f;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Replicated)
	float PreviousDelta = 0.0f;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Replicated)
	float ForceMag = 0.0f;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Replicated)
	FVector Force = FVector(0, 0, 0);

	UFUNCTION(Server, Reliable)
	void SetGrapplePointServer(FVector GrapplePoint, AActor* HitActor);

	bool IsGrapplePointValid(FVector GrapplePoint);

	void RegisterNewGrapplePoint(FVector GrapplePoint, AActor* HitActor);

	void ApplyReel(float DeltaSeconds);

	void ApplyGrapple();

	UFUNCTION(Client, Unreliable)
	void ClientApplyGrappleForce(FVector ForceToApply);

	void StopGrappleLocal();

	UFUNCTION(Server, Reliable)
	void StopGrappleRemote();

	void SetRopeLengthLocal(float AxisValue);

	void RegisterNewRopeLength(float AxisValue);

	UFUNCTION(Server, Unreliable)
	void RegisterNewRopeLengthServer(float AxisValue);

	void UpdateRopeLength(float NewRopeLength);

	UFUNCTION(Server, Unreliable)
	void UpdateRopeLengthServer(float NewRopeLength);

	void SetGrappleBoostLocal();

	void BoostPlayer();

	UFUNCTION(Server, Reliable)
	void BoostPlayerServer();

private:
	/*
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Replicated)
	float Delta = 0.0f;
	UPROPERTY(Replicated)
	float PreviousDelta = 0.0f;
	UPROPERTY(Replicated)
	float ForceMag = 0.0f;
	*/
	//Replication
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
