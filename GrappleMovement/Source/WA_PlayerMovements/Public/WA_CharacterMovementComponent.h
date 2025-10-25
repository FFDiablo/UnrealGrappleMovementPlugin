// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AHookPoint.h"
#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WA_CharacterMovementComponent.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None			UMETA(Hidden),
	CMOVE_Grapple		UMETA(DisplayName = "Grapple"),
	CMOVE_Glide			UMETA(DisplayName = "Glide"),
	CMOVE_Climb			UMETA(DisplayName = "Climb"),
	CMOVE_MAX			UMETA(Hidden),
};


UCLASS()
class WA_PLAYERMOVEMENTS_API UWA_CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

		class FSavedMove_WA : public FSavedMove_Character
		{
		public:
			enum CompressedFlags
			{
				FLAG_Sprint = 0x10,
				FLAG_Walk = 0x20,
				FLAG_Custom_2 = 0x40,
				FLAG_Custom_3 = 0x80,
			};

			uint8 Saved_bWantToSprint : 1;
			uint8 Saved_bWantToWalk : 1;

			uint8 Saved_bWantToGrapple : 1;
			float Saved_fGrappleReel;

			// Other Variables
			uint8 Saved_bHadAnimRootMotion : 1;
			uint8 Saved_bTransitionFinished : 1;
			
			FSavedMove_WA();
			
			virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
			virtual void Clear() override;
			virtual uint8 GetCompressedFlags() const override;
			virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
			virtual void PrepMoveFor(ACharacter* C) override;
		};

		class FNetworkPredictionData_Client_WA : public FNetworkPredictionData_Client_Character
		{
			public:
				FNetworkPredictionData_Client_WA(const UCharacterMovementComponent& ClientMovement);

				typedef FNetworkPredictionData_Client_Character Super;

				virtual FSavedMovePtr AllocateNewMove() override;
		};

		//Ground Movements Parameters
		UPROPERTY(EditDefaultsOnly, Category = "Ground Movements Parameters") float Sprint_MaxWalkSpeed = 1000.f;
		UPROPERTY(EditDefaultsOnly, Category = "Ground Movements Parameters") float Jog_MaxWalkSped = 650.0f;
		UPROPERTY(EditDefaultsOnly, Category = "Ground Movements Parameters") float Walk_MaxWalkSpeed = 350.f;

		//Climb Movements Parameters
		UPROPERTY(EditDefaultsOnly, Category = "Climb Movements Parameters") float MaxClimbSpeed = 300.f;
		UPROPERTY(EditDefaultsOnly, Category = "Climb Movements Parameters") float BrakingDecelerationClimbing = 1000.f;
		UPROPERTY(EditDefaultsOnly, Category = "Climb Movements Parameters") float ClimbReachDistance = 200.f;
		UPROPERTY(EditDefaultsOnly, Category = "Climb Movements Parameters") UAnimMontage* WallJumpMontage;
		UPROPERTY(EditDefaultsOnly, Category = "Climb Movements Parameters") float WallJumpForce = 400.f;

		//UPROPERTY(Replicated) ACharacter* Ref_OwnerCharacter;

		//Transient
		UPROPERTY(Transient) AWA_Character* WACharacterOwner;

		bool Safe_bHadAnimRootMotion;
		bool Safe_bTransitionFinished;
		int TransitionRMS_ID;

	//Flags 
	bool Safe_bWantToSprint;
	bool Safe_bWantToWalk;
	bool Safe_bWantToGrapple;
	float Safe_fGrappleReel;

	//Network Properties
	float AccumulatedClientLocationError = 0.f;

	int TickCount = 0;
	int CorrectionCount = 0;
	int TotalBitsSent = 0;

public :
	UWA_CharacterMovementComponent();
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Actor Component
protected:
	virtual void InitializeComponent() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) bool IsDebug = false;

//Character Movement Component Part :
protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnClientCorrectionReceived(FNetworkPredictionData_Client_Character& ClientData, float TimeStamp, FVector NewLocation, FVector NewVelocity, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, FVector ServerGravityDirection) override;

public:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

protected:
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual bool ServerCheckClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) override;
	
	FNetBitWriter WAServerMoveBitWriter;

	virtual void CallServerMovePacked(const FSavedMove_Character* NewMove, const FSavedMove_Character* PendingMove, const FSavedMove_Character* OldMove) override;


	//Grapple Movement Functions
private:
	//bool GetHookPoints(FHitResult& Hit) const;
	void PhysGrapple(float deltaTime, int32 Iterations);

//Interfaces
public : 
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();

	UFUNCTION(BlueprintCallable) void WalkPressed();
	UFUNCTION(BlueprintCallable) void WalkReleased();

	UFUNCTION(BlueprintCallable) void CrouchPressed();

	UFUNCTION(BlueprintCallable) void EnterGrapple();
	UFUNCTION(BlueprintCallable) void ExitGrapple();

	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	UFUNCTION(BlueprintPure) bool IsMovementMode(EMovementMode InMovementMode) const;


//Getters
public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;

	virtual bool CanAttemptJump() const override;

// Helpers Functions
private:
	bool IsServer() const;
	float CapR() const;
	float CapHH() const;


};
