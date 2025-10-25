// Fill out your copyright notice in the Description page of Project Settings.


#include "WA_Character.h"

#include "WA_CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"

#include "EnhancedInputComponent.h"        // for UEnhancedInputComponent & BindAction
#include "EnhancedInputSubsystems.h"       // for UEnhancedInputLocalPlayerSubsystem
#include "InputMappingContext.h"           // for UInputMappingContext
#include "InputAction.h"                   // for UInputAction
#include "InputActionValue.h"              // for FInputActionValue
//#include "Engine/LocalPlayer.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Math/Vector.h"
#include <Net/UnrealNetwork.h>
#include <Kismet/KismetMathLibrary.h>

// Sets default values
AWA_Character::AWA_Character(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UWA_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	WA_CharacterMovementComponent = Cast <UWA_CharacterMovementComponent>(GetCharacterMovement());

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	TurnRateGamepad = 50.f;

	bUseControllerRotationPitch = bUseControllerRotationYaw = bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = true;
	//GetCharacterMovement()->bServerAcceptClientAuthoritativePosition = true;
}



void AWA_Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsGrappling)
	{
		FVector ComponentLocation_t;
		if (IsValid(Ref_HookPoint)) {
			ComponentLocation_t = Ref_HookPoint->RopeAttachLocation->GetComponentLocation();
		}
		else {
			ComponentLocation_t = CurrentGrapplePoint;
		}

		FVector grappleDefaultPlayerPos_t = ComponentLocation_t - FVector(0.0f, 0.0f, RopeLength);

		float DistPlayerToHook_t = UKismetMathLibrary::Vector_Distance(GetActorLocation(), ComponentLocation_t);
		float DistHookToInitial_t = UKismetMathLibrary::Vector_Distance(ComponentLocation_t, grappleDefaultPlayerPos_t);

		bool Check = DistPlayerToHook_t >= DistHookToInitial_t;

		ApplyReel(DeltaSeconds);

		if (Check == true) {

			ApplyGrapple();
			GEngine->AddOnScreenDebugMessage(35, 1.f, FColor::Green, FString::Printf(TEXT("Applying Grapple Physic !")));
		}
	}
}

// Called to bind functionality to input
void AWA_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (!bUseOldInputs) {
		// Set up action bindings
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
			UE_LOG(LogTemp, Log, TEXT("[EI] Using EnhancedInputComponent"));
			// Jumping
			EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

			// Moving
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AWA_Character::MoveAction);

			// Looking
			EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AWA_Character::LookAction);
			EnhancedInputComponent->BindAction(IA_MouseLook, ETriggerEvent::Triggered, this, &AWA_Character::LookAction);


		}else
		{
			//UE_LOG(ScreenLog, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
			GEngine->AddOnScreenDebugMessage(2, 100.f, FColor::Yellow, FString::Printf(TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this)));
		}
	}else {
			check(PlayerInputComponent);
			PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
			PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

			PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AWA_Character::MoveForward);
			PlayerInputComponent->BindAxis("Move Right / Left", this, &AWA_Character::MoveRight);

			PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
			PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AWA_Character::TurnAtRate);
			PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
			PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AWA_Character::LookUpAtRate);

			PlayerInputComponent->BindTouch(IE_Pressed, this, &AWA_Character::TouchStarted);
			PlayerInputComponent->BindTouch(IE_Released, this, &AWA_Character::TouchStopped);

			GEngine->AddOnScreenDebugMessage(2, 100.f, FColor::Yellow, FString::Printf(TEXT("Using Old Inputs")));
			return;
	}
	
}

void AWA_Character::MoveAction(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AWA_Character::LookAction(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AWA_Character::TurnAtRateAction(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	TurnAtRate(AxisValue);
}

void AWA_Character::LookUpAtRateAction(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	LookUpAtRate(AxisValue);
}



void AWA_Character::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AWA_Character::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}


void AWA_Character::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		if (!bIsGrappling)
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);
		}
		else {
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value*2);
		}
	}
}

void AWA_Character::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		if (!bIsGrappling)
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			AddMovementInput(Direction, Value);
		}
		else {
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			AddMovementInput(Direction, Value*2);
		}
		

	}
}

void AWA_Character::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AWA_Character::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AWA_Character::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AWA_Character::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

/* This part is for the Grapple if this work I'm gonna do something that is not allowed by the Catholic Church
Go dammit I really hope this fucking work because I really want to player with floaty boat and go brbrbrbrbrbbr with my friends :c*/

void AWA_Character::GrapplePressed()
{
	TrySetGrappleLocal();
}

void AWA_Character::GrappleReleased()
{
	StopGrappleLocal();
}

void AWA_Character::GrappleReel(float AxisValue)
{
	if (bIsGrappling)
	{
		SetRopeLengthLocal(AxisValue);
		GEngine->AddOnScreenDebugMessage(17, 1.f, FColor::Purple, FString::Printf(TEXT("Reeling !")));
	}
}

void AWA_Character::ReleasedGrappleReel()
{
	if (bIsGrappling)
	{
		//RegisterNewRopeLength(BufferRopeLength);
		UpdateRopeLengthServer(RopeLength);
	}
}

void AWA_Character::BoostGrapple()
{
	if (bIsGrappling)
	{
		//Boost 
		SetGrappleBoostLocal();
	}
}



void AWA_Character::TrySetGrappleLocal()
{
	UCameraComponent* Cam = GetFollowCamera();
	FVector Start = Cam->GetComponentLocation();
	float TraceMaxDistance = 10000.f;
	FVector End = Start + Cam->GetForwardVector() * TraceMaxDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult OutHit;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = OutHit.GetActor();
		
		SetGrapplePointLocal(OutHit.ImpactPoint, HitActor);
	}
}

void AWA_Character::SetGrapplePointLocal(FVector GrapplePoint, AActor* HitActor)
{
	RegisterNewGrapplePoint(GrapplePoint, HitActor);
	SetGrapplePointServer(GrapplePoint, HitActor);
}

void AWA_Character::SetGrapplePointServer_Implementation(FVector GrapplePoint, AActor* HitActor)
{
	RegisterNewGrapplePoint(GrapplePoint, HitActor);
}



bool AWA_Character::IsGrapplePointValid(FVector GrapplePoint)
{

	//Need to add some Server Side validation to block some cheat options
	FVector GrappleDiff = GrapplePoint - GetActorLocation();
	if (GrappleDiff.Length() > MaxGrappleDistance)
	{
		return false;
	}

	//Need to add a check if there is something between the player here to add rope collision / Physic
	return true;
}

void AWA_Character::RegisterNewGrapplePoint(FVector GrapplePoint, AActor* HitActor)
{
	if (IsGrapplePointValid(GrapplePoint))
	{
		CurrentGrapplePoint = GrapplePoint;
		CurrentGrappleDistance = (GrapplePoint - GetActorLocation()).Length();

		RopeLength = FMath::Clamp(((ClimbSpeed * 0) + CurrentGrappleDistance), 150.0f, MaxGrappleDistance);
		RopeLength = FMath::RoundHalfToZero(RopeLength);
		RopeLength = RopeLength - 46.0f;
		UpdateRopeLength(RopeLength);
		UpdateRopeLengthServer(RopeLength);
		bIsGrappling = true;

		//WA_CharacterMovementComponent->EnterGrapple();


		if (HasAuthority()) {

			//Spawn hookpoint
			FActorSpawnParameters SpawnInfo;
			//FRotator RotationX = UKismetMathLibrary::MakeRotFromX(OutHit.ImpactNormal);
			FRotator RotationX(0.0f, 0.0f, 0.0f);

			Ref_HookPoint = GetWorld()->SpawnActor<AAHookPoint>(HookToSpawn, GrapplePoint, RotationX, SpawnInfo);
			Ref_HookPoint->SetOwner(this);
			Ref_HookPoint->AttachToActor(HitActor, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
}

void AWA_Character::ApplyReel(float DeltaSeconds)
{
	if (!bIsGrappling) { return; }

	float bufropelength = RopeLength;

	//if (HasAuthority()) {

		if (ReelIn || ReelOut) {
			if (ReelIn) {
				bufropelength = FMath::Clamp(((bufropelength - (20 * (ClimbSpeed * (DeltaSeconds * 2))))), 250.0f, MaxGrappleDistance);
				bufropelength = FMath::RoundHalfToZero(bufropelength);

				UpdateRopeLength(bufropelength);
				UpdateRopeLengthServer(bufropelength);


				GEngine->AddOnScreenDebugMessage(20, 1.f, FColor::Blue, FString::Printf(TEXT("DeltaSeconds : %f"), DeltaSeconds));
				GEngine->AddOnScreenDebugMessage(15, 1.f, FColor::Purple, FString::Printf(TEXT("Reeling In !")));
			}
			if (ReelOut) {
				//bufropelength = FMath::Clamp(((ClimbSpeed * 3) + bufropelength), 100.0f, MaxGrappleDistance);
				//bufropelength = FMath::RoundHalfToZero(bufropelength);

				bufropelength = FMath::Clamp(((bufropelength + (20 * (ClimbSpeed * (DeltaSeconds * 4))))), 250.0f, MaxGrappleDistance);
				bufropelength = FMath::RoundHalfToZero(bufropelength);

				UpdateRopeLength(bufropelength);
				UpdateRopeLengthServer(bufropelength);

				GEngine->AddOnScreenDebugMessage(16, 1.f, FColor::Purple, FString::Printf(TEXT("Reeling Out !")));
			}
		}

	//}
}

void AWA_Character::ApplyGrapple()
{
	if (!bIsGrappling) { return; }

	if (UseNewGrapplePhys){

	FVector RopeAttachPointLocation;
	FVector DirPlayerToHook;

	if (bIsGrappling) {

		if (IsValid(Ref_HookPoint)) {

			RopeAttachPointLocation = Ref_HookPoint->RopeAttachLocation->GetComponentLocation();
			HookLocationOffset = Ref_HookPoint->RopeAttachLocation->GetComponentLocation() - FVector(0.0f, 0.0f, 75.0f);
			DirPlayerToHook = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), Ref_HookPoint->RopeAttachLocation->GetComponentLocation());
		}
		else {
			//Need to change to the last array item of hookpoints array
			RopeAttachPointLocation = CurrentGrapplePoint;
			HookLocationOffset = CurrentGrapplePoint - FVector(0.0f, 0.0f, 75.0f);
			DirPlayerToHook = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), CurrentGrapplePoint);
		}

	}

	FVector grappleDefaultPlayerPos = RopeAttachPointLocation - FVector(0.0f, 0.0f, RopeLength);

	float DistPlayerToHook = UKismetMathLibrary::Vector_Distance(GetActorLocation(), HookLocationOffset);
	float DistHookToInitial = UKismetMathLibrary::Vector_Distance(HookLocationOffset, grappleDefaultPlayerPos);

	Delta = DistPlayerToHook - DistHookToInitial;

	ForceMag = (Delta * SpringForce) + (SpringForceDamping * ((Delta - PreviousDelta) / GetWorld()->GetDeltaSeconds()));

	Force = DirPlayerToHook * ForceMag;
	
	GetCharacterMovement()->AddForce(Force);
	
	
		
	PreviousDelta = Delta;

	FVector OldPlayerVelocity = GetCharacterMovement()->Velocity;
	FVector NewPlayerVelocity = UKismetMathLibrary::VInterpTo(OldPlayerVelocity, FVector(0.0f, 0.0f, 0.0f), GetWorld()->GetDeltaSeconds(), VelocityDamping);
	GetCharacterMovement()->Velocity = NewPlayerVelocity;

	}
	else {

		FVector GrappleDiff;

		if (IsValid(Ref_HookPoint)) {
			GrappleDiff = Ref_HookPoint->RopeAttachLocation->GetComponentLocation() - GetActorLocation();
		}
		else {
			GrappleDiff = CurrentGrapplePoint - GetActorLocation();
		}

	
	//Slack Rope

	//if (GrappleDiff.Length() < CurrentGrappleDistance) { return; }

	FVector Vel = GetVelocity();

	FVector NormalTowardsGrapplePoint = GrappleDiff.GetSafeNormal();
	FVector VelNormal = Vel.GetSafeNormal();

	float AngleBetweenVelocityAndGrapple = UKismetMathLibrary::DegAcos(VelNormal.Dot(NormalTowardsGrapplePoint));
	if (AngleBetweenVelocityAndGrapple < 90) { return; }

	FVector NewVelocity = UKismetMathLibrary::ProjectVectorOnToPlane(Vel, NormalTowardsGrapplePoint);

	float MaxSpeed = 10000.0f;
	if (NewVelocity.SizeSquared() > FMath::Square(MaxSpeed))
	{
		FVector ClampedVelocityDirection = NewVelocity.GetSafeNormal();
		NewVelocity = ClampedVelocityDirection * MaxSpeed;
	}


	LaunchCharacter(NewVelocity, true, true);

	GEngine->AddOnScreenDebugMessage(36, 1.f, FColor::Purple, FString::Printf(TEXT("Old Grapple Phys Applying !")));
	}
}


void AWA_Character::ClientApplyGrappleForce_Implementation(FVector ForceToApply)
{
	GetCharacterMovement()->AddForce(Force);
}

void AWA_Character::StopGrappleLocal()
{
	bIsGrappling = false;
	StopGrappleRemote();
	//WA_CharacterMovementComponent->ExitGrapple();
}

void AWA_Character::StopGrappleRemote_Implementation()
{
	bIsGrappling = false;
}

void AWA_Character::SetRopeLengthLocal(float AxisValue)
{
	RegisterNewRopeLength(AxisValue);
	RegisterNewRopeLengthServer(AxisValue);
}


void AWA_Character::RegisterNewRopeLength(float AxisValue)
{
	if (AxisValue == -1.0f) {
		ReelIn = true;
		GEngine->AddOnScreenDebugMessage(15, 1.f, FColor::Purple, FString::Printf(TEXT("Reel In True !")));
		//ApplyReel();
	}
	else {
		ReelIn = false;
	}

	if (AxisValue == 1.0f) {
		ReelOut = true;
		GEngine->AddOnScreenDebugMessage(16, 1.f, FColor::Purple, FString::Printf(TEXT("Reel Out True !")));
		//ApplyReel();
	}
	else {
		ReelOut = false;
	}
}

void AWA_Character::RegisterNewRopeLengthServer_Implementation(float AxisValue)
{
	RegisterNewRopeLength(AxisValue);
}

void AWA_Character::UpdateRopeLength(float NewRopeLength)
{
	RopeLength = NewRopeLength;
}

void AWA_Character::UpdateRopeLengthServer_Implementation(float NewRopeLength)
{
	UpdateRopeLength(NewRopeLength);
}

void AWA_Character::SetGrappleBoostLocal()
{
	BoostPlayer();
	BoostPlayerServer();

}

void AWA_Character::BoostPlayer()
{
	FVector BoostVelocity = ClampVector(1.5*GetCharacterMovement()->Velocity, FVector(-2500, -2500, -2500), FVector(2500, 2500, 2500));
	/*
	if (BoostVelocity.X < 1500 && BoostVelocity.Y < 1500 && BoostVelocity.Z < 1500)
	{

		FVector HoorDirfromPlayer = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), CurrentGrapplePoint);
		BoostVelocity = ClampVector(HoorDirfromPlayer * 2.5, FVector(-2500, -2500, -2500), FVector(2500, 2500, 2500));
	}
	*/
	LaunchCharacter(BoostVelocity, true, true);

	bIsGrappling = false;
	StopGrappleRemote();
}

void AWA_Character::BoostPlayerServer_Implementation()
{
	BoostPlayer();
}

//Replication def
void AWA_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWA_Character, bIsGrappling);
	DOREPLIFETIME(AWA_Character, CurrentGrapplePoint);
	DOREPLIFETIME(AWA_Character, CurrentGrappleDistance);
	DOREPLIFETIME_CONDITION(AWA_Character, RopeLength, COND_SkipOwner);
	DOREPLIFETIME(AWA_Character, ReelIn);
	DOREPLIFETIME(AWA_Character, ReelOut);
	DOREPLIFETIME(AWA_Character, UseNewGrapplePhys);
	DOREPLIFETIME(AWA_Character, SpringForce);
	DOREPLIFETIME(AWA_Character, SpringForceDamping);
	DOREPLIFETIME(AWA_Character, VelocityDamping);
	DOREPLIFETIME(AWA_Character, HookToSpawn);
	DOREPLIFETIME(AWA_Character, Ref_HookPoint);
	DOREPLIFETIME_CONDITION(AWA_Character, Delta, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWA_Character, PreviousDelta, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWA_Character, ForceMag, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AWA_Character, Force, COND_SkipOwner);
}