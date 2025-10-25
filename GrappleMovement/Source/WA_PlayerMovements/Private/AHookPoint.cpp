// Fill out your copyright notice in the Description page of Project Settings.

#include "AHookPoint.h"

// Sets default values
AAHookPoint::AAHookPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	SetReplicateMovement(true);

	Scene = CreateDefaultSubobject<USceneComponent>("Root Component");
	HookMesh = CreateDefaultSubobject<UStaticMeshComponent>("Hook Mesh");
	RopeAttachLocation = CreateDefaultSubobject<USceneComponent>("Rope Attach Location");

	RootComponent = Scene;
	
	Scene->SetMobility(EComponentMobility::Movable);
	
	HookMesh->SetupAttachment(Scene);
	RopeAttachLocation->SetupAttachment(HookMesh);

}

// Called when the game starts or when spawned
void AAHookPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAHookPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

