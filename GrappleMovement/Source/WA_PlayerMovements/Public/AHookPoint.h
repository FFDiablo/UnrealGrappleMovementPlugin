// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

#include "GameFramework/Actor.h"
#include "AHookPoint.generated.h"

UCLASS()
class WA_PLAYERMOVEMENTS_API AAHookPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAHookPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Hook Point")
		class USceneComponent* Scene;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Hook Point")
		class UStaticMeshComponent* HookMesh;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Hook Point")
		class USceneComponent* RopeAttachLocation;
};
