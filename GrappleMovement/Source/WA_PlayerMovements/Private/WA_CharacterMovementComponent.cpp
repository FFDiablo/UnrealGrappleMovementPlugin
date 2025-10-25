// Fill out your copyright notice in the Description page of Project Settings.


#include "WA_CharacterMovementComponent.h"

#include "WA_Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Net/UnrealNetwork.h"

#pragma region ComponentSetup
UWA_CharacterMovementComponent::UWA_CharacterMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
	WAServerMoveBitWriter.SetAllowResize(true);
}

void UWA_CharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickCount++;
	if (IsNetMode(NM_Client))
	{
		if (IsDebug)
		{
			GEngine->AddOnScreenDebugMessage(2, 100.f, FColor::Yellow, FString::Printf(TEXT("Correction: %.2f"), 100.f * (float)CorrectionCount / (float)TickCount));
			GEngine->AddOnScreenDebugMessage(9, 100.f, FColor::Yellow, FString::Printf(TEXT("Bitrate: %.3f"), (float)TotalBitsSent / GetWorld()->GetTimeSeconds() / 1000.f));
		}
	}
	else
	{
		if (IsDebug)
		{
			GEngine->AddOnScreenDebugMessage(3, 100.f, FColor::Yellow, FString::Printf(TEXT("Location Error: %.4f cm/s"), 100.f * AccumulatedClientLocationError / GetWorld()->GetTimeSeconds()));
		}
	}
}


void UWA_CharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	WACharacterOwner = Cast<AWA_Character>(GetOwner());
	//Ref_OwnerCharacter = Cast<ACharacter>(GetOwner());
}

#pragma endregion

#pragma region FSavedMove

//Check if the Prediction Data is valid and if not create it.
FNetworkPredictionData_Client* UWA_CharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

		if (ClientPredictionData == nullptr)
		{
			UWA_CharacterMovementComponent* MutableThis = const_cast<UWA_CharacterMovementComponent*>(this);

			MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_WA(*this);
			MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
			MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
		}
	return ClientPredictionData;
}


UWA_CharacterMovementComponent::FSavedMove_WA::FSavedMove_WA()
{
	Saved_bWantToSprint = 0;
}

//Check two move to save bandwidth and skip if too similar (True if similar)
bool UWA_CharacterMovementComponent::FSavedMove_WA::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_WA* NewWAMove = static_cast<FSavedMove_WA*>(NewMove.Get());

	if (Saved_bWantToSprint != NewWAMove->Saved_bWantToSprint)
	{
		return false;
	}
	if (Saved_bWantToWalk != NewWAMove->Saved_bWantToWalk)
	{
		return false;
	}

	if (Saved_bWantToGrapple != NewWAMove->Saved_bWantToGrapple)
	{
		return false;
	}

	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}


//Reset "SavedMove_WA" to be empty.
void UWA_CharacterMovementComponent::FSavedMove_WA::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantToSprint = 0;
	Saved_bWantToWalk = 0;

	Saved_bWantToGrapple = 0;
	Saved_fGrappleReel = 0.0f;

	Saved_bHadAnimRootMotion = 0;
	Saved_bTransitionFinished = 0;
}

//Compressed packet to save bandwidth.
uint8 UWA_CharacterMovementComponent::FSavedMove_WA::GetCompressedFlags() const
{
	uint8 Result = FSavedMove_Character::GetCompressedFlags();

	if (Saved_bWantToSprint) Result |= FLAG_Sprint;

	if (Saved_bWantToWalk) Result |= FLAG_Walk;

	return Result;
}


//Capture Player Character current "state"
void UWA_CharacterMovementComponent::FSavedMove_WA::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const UWA_CharacterMovementComponent* CharacterMovement = Cast<UWA_CharacterMovementComponent>(C->GetCharacterMovement());

	Saved_bWantToSprint = CharacterMovement->Safe_bWantToSprint;
	Saved_bWantToWalk = CharacterMovement->Safe_bWantToWalk;

	Saved_bWantToGrapple = CharacterMovement->Safe_bWantToGrapple;
	Saved_fGrappleReel = CharacterMovement->Safe_fGrappleReel;

	Saved_bHadAnimRootMotion = CharacterMovement->Safe_bHadAnimRootMotion;
	Saved_bTransitionFinished = CharacterMovement->Safe_bTransitionFinished;
}

void UWA_CharacterMovementComponent::FSavedMove_WA::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	UWA_CharacterMovementComponent* CharacterMovement = Cast<UWA_CharacterMovementComponent>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantToSprint = Saved_bWantToSprint;
	CharacterMovement->Safe_bWantToWalk = Saved_bWantToWalk;

	CharacterMovement->Safe_bWantToGrapple = Saved_bWantToGrapple;
	CharacterMovement->Safe_fGrappleReel = Saved_fGrappleReel;

	CharacterMovement->Safe_bHadAnimRootMotion = Saved_bHadAnimRootMotion;
	CharacterMovement->Safe_bTransitionFinished = Saved_bTransitionFinished;
}

#pragma endregion

#pragma region ClientPredictionData

UWA_CharacterMovementComponent::FNetworkPredictionData_Client_WA::FNetworkPredictionData_Client_WA(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
	//Nothing to change here ._.
}

FSavedMovePtr UWA_CharacterMovementComponent::FNetworkPredictionData_Client_WA::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_WA());
}

#pragma endregion

#pragma region CharacterMovementComponent

void UWA_CharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantToSprint = (Flags & FSavedMove_WA::FLAG_Sprint) != 0;
	Safe_bWantToWalk = (Flags & FSavedMove_WA::FLAG_Walk) != 0;
}

void UWA_CharacterMovementComponent::OnClientCorrectionReceived(FNetworkPredictionData_Client_Character& ClientData, float TimeStamp, FVector NewLocation, FVector NewVelocity, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, FVector ServerGravityDirection)
{
	Super::OnClientCorrectionReceived(ClientData, TimeStamp, NewLocation, NewVelocity, NewBase, NewBaseBoneName,bHasBase, bBaseRelativePosition, ServerMovementMode, ServerGravityDirection);

	CorrectionCount++;
}

void UWA_CharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	//Switch Grapple Movement Mode
	if (Safe_bWantToGrapple)
	{
		SetMovementMode(MOVE_Custom, CMOVE_Grapple);
	}
	else {
		//SetMovementMode(MOVE_Walking);
	}


	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UWA_CharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);

	if (!HasAnimRootMotion() && Safe_bHadAnimRootMotion && IsMovementMode(MOVE_Flying))
	{
		UE_LOG(LogTemp, Warning, TEXT("Ending Anim Root Motion"))
			SetMovementMode(MOVE_Walking);
	}

	if (GetRootMotionSourceByID(TransitionRMS_ID) && GetRootMotionSourceByID(TransitionRMS_ID)->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
	{
		RemoveRootMotionSourceByID(TransitionRMS_ID);
		Safe_bTransitionFinished = true;
	}


	Safe_bHadAnimRootMotion = HasAnimRootMotion();
}

void UWA_CharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{

	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (MovementMode == MOVE_Walking) {
		if (Safe_bWantToSprint && !Safe_bWantToWalk)
		{
			MaxWalkSpeed = Sprint_MaxWalkSpeed;
			return;
		}
		if (Safe_bWantToWalk && !Safe_bWantToSprint)
		{
			MaxWalkSpeed = Walk_MaxWalkSpeed;
			return;
		}
		if(!Safe_bWantToSprint && !Safe_bWantToWalk)
		{
			MaxWalkSpeed = Jog_MaxWalkSped;
			return;
		}
		
	}
}


void UWA_CharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Grapple:
		PhysGrapple(deltaTime, Iterations);
		break;
	case CMOVE_Glide:
		//PhysGlide(deltaTime, Iterations);
		UE_LOG(LogTemp, Fatal, TEXT("CMOVE_Glide  Not implemented yet!"))
		break;
	case CMOVE_Climb:
		//PhysClimb(deltaTime, Iterations);
		UE_LOG(LogTemp, Fatal, TEXT("CMOVE_Climb  Not implemented yet!"))
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
	};
}

void UWA_CharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (IsFalling())
	{
		bOrientRotationToMovement = true;
	}
}

bool UWA_CharacterMovementComponent::ServerCheckClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	if (GetCurrentNetworkMoveData()->NetworkMoveType == FCharacterNetworkMoveData::ENetworkMoveType::NewMove)
	{
		float LocationError = FVector::Dist(UpdatedComponent->GetComponentLocation(), ClientWorldLocation);
		GEngine->AddOnScreenDebugMessage(6, 100.f, FColor::Yellow, FString::Printf(TEXT("Loc: %s"), *ClientWorldLocation.ToString()));
		AccumulatedClientLocationError += LocationError * DeltaTime;
	}



	return Super::ServerCheckClientError(ClientTimeStamp, DeltaTime, Accel, ClientWorldLocation, RelativeClientLocation,
		ClientMovementBase,
		ClientBaseBoneName, ClientMovementMode);

}

void UWA_CharacterMovementComponent::CallServerMovePacked(const FSavedMove_Character* NewMove, const FSavedMove_Character* PendingMove, const FSavedMove_Character* OldMove)
{
	// Get storage container we'll be using and fill it with movement data
	FCharacterNetworkMoveDataContainer& MoveDataContainer = GetNetworkMoveDataContainer();
	MoveDataContainer.ClientFillNetworkMoveData(NewMove, PendingMove, OldMove);

	// Reset bit writer without affecting allocations
	FBitWriterMark BitWriterReset;
	BitWriterReset.Pop(WAServerMoveBitWriter);

	// 'static' to avoid reallocation each invocation
	static FCharacterServerMovePackedBits PackedBits;
	UNetConnection* NetConnection = CharacterOwner->GetNetConnection();


	{
		// Extract the net package map used for serializing object references.
		WAServerMoveBitWriter.PackageMap = NetConnection ? ToRawPtr(NetConnection->PackageMap) : nullptr;
	}

	if (WAServerMoveBitWriter.PackageMap == nullptr)
	{
		UE_LOG(LogNetPlayerMovement, Error, TEXT("CallServerMovePacked: Failed to find a NetConnection/PackageMap for data serialization!"));
		return;
	}

	// Serialize move struct into a bit stream
	if (!MoveDataContainer.Serialize(*this, WAServerMoveBitWriter, WAServerMoveBitWriter.PackageMap) || WAServerMoveBitWriter.IsError())
	{
		UE_LOG(LogNetPlayerMovement, Error, TEXT("CallServerMovePacked: Failed to serialize out movement data!"));
		return;
	}

	// Copy bits to our struct that we can NetSerialize to the server.
	PackedBits.DataBits.SetNumUninitialized(WAServerMoveBitWriter.GetNumBits());

	check(PackedBits.DataBits.Num() >= WAServerMoveBitWriter.GetNumBits());
	FMemory::Memcpy(PackedBits.DataBits.GetData(), WAServerMoveBitWriter.GetData(), WAServerMoveBitWriter.GetNumBytes());

	TotalBitsSent += PackedBits.DataBits.Num();

	// Send bits to server!
	ServerMovePacked_ClientSend(PackedBits);

	MarkForClientCameraUpdate();
}
#pragma endregion

#pragma region Phys

void UWA_CharacterMovementComponent::PhysGrapple(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	RestorePreAdditiveRootMotionVelocity();

	// Velocity += Safe_vThrust;

	float timeTick = GetSimulationTimeStep(DeltaTime, Iterations);

	// Add Thrust Velocity To Component
	// Velocity = Velocity + Safe_vThrust;

	// Time Acceleration Gravity
	FVector FallAcceleration = GetFallingLateralAcceleration(DeltaTime);
	FallAcceleration.Z = 0.f;

	// Compute current gravity
	const FVector Gravity(0.f, 0.f, GetGravityZ());
	float GravityTime = timeTick;

	// Apply gravity
	Velocity = NewFallVelocity(Velocity, Gravity, GravityTime);

	// Calc Velocity
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		//const float Friction = 0.5f * UpdatedComponent->GetPhysicsVolume()->FluidFriction;
		//CalcVelocity(DeltaTime, Friction, true, GetMaxBrakingDeceleration());
		// CalcVelocity(deltaTime, Move_Friction, true, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(DeltaTime);

	float remainingTime = DeltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		// Perform Move
		Iterations++;
		bJustTeleported = false;
		FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FVector Adjusted = Velocity * DeltaTime;
		FHitResult Hit(1.f);

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;
		bool bFloorWalkable = CurrentFloor.IsWalkableFloor();

		// SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);
		SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);
		if (Hit.Time < 1.f)
		{
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}
	}
}


#pragma endregion

#pragma region Interfaces

void UWA_CharacterMovementComponent::SprintPressed()
{
	Safe_bWantToSprint = true;
}

void UWA_CharacterMovementComponent::SprintReleased()
{
	Safe_bWantToSprint = false;
}

void UWA_CharacterMovementComponent::WalkPressed()
{
	Safe_bWantToWalk = true;
}

void UWA_CharacterMovementComponent::WalkReleased()
{
	Safe_bWantToWalk = false;
}

void UWA_CharacterMovementComponent::CrouchPressed()
{
	bWantsToCrouch = !bWantsToCrouch;
}

void UWA_CharacterMovementComponent::EnterGrapple()
{
	GEngine->AddOnScreenDebugMessage(150, 15.f, FColor::Red, FString::Printf(TEXT("Enter Grapple CMC !")));
	Safe_bWantToGrapple = true;
}

void UWA_CharacterMovementComponent::ExitGrapple()
{
	GEngine->AddOnScreenDebugMessage(150, 15.f, FColor::Red, FString::Printf(TEXT("EXIT Grapple CMC !")));
	Safe_bWantToGrapple = false;
}



bool UWA_CharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

bool UWA_CharacterMovementComponent::IsMovementMode(EMovementMode InMovementMode) const
{
	return InMovementMode == MovementMode;
}

#pragma endregion

#pragma region Getters

bool UWA_CharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Climb);
}

bool UWA_CharacterMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

float UWA_CharacterMovementComponent::GetMaxSpeed() const
{
	if (IsMovementMode(MOVE_Walking) && Safe_bWantToSprint && !IsCrouching()) return Sprint_MaxWalkSpeed;
	if (IsMovementMode(MOVE_Walking) && Safe_bWantToWalk && !IsCrouching()) return Walk_MaxWalkSpeed;

	if (MovementMode != MOVE_Custom) return Super::GetMaxSpeed();

	switch (CustomMovementMode)
	{
	case CMOVE_Grapple:
		return 5000.f;
	case CMOVE_Glide:
		return 0.f;
	case CMOVE_Climb:
		return MaxClimbSpeed;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
			return -1.f;
	}
}

float UWA_CharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (MovementMode != MOVE_Custom) return Super::GetMaxBrakingDeceleration();

	switch (CustomMovementMode)
	{
	case CMOVE_Grapple:
		return 0.f;
	case CMOVE_Glide:
		return 0.f;
	case CMOVE_Climb:
		return BrakingDecelerationClimbing;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
			return -1.f;
	}
}

bool UWA_CharacterMovementComponent::CanAttemptJump() const
{
	return Super::CanAttemptJump();
}

#pragma endregion

#pragma region Helpers

bool UWA_CharacterMovementComponent::IsServer() const
{
	return CharacterOwner->HasAuthority();
}

float UWA_CharacterMovementComponent::CapR() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UWA_CharacterMovementComponent::CapHH() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

#pragma endregion