// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsEnv.h"
#include "GameFramework/Actor.h"
#include "FrameManager.generated.h"

UCLASS()
class ANX_5_2_0_DOG_R_1_3_API AFrameManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFrameManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable)
    void OnStart();

    TSharedPtr<puerts::FJsEnv> JsEnv;

};
