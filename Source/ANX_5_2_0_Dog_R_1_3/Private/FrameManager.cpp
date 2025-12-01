// Fill out your copyright notice in the Description page of Project Settings.


#include "FrameManager.h"


// Sets default values
AFrameManager::AFrameManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFrameManager::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AFrameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AFrameManager::OnStart()
{
    // GameScript = MakeShared<puerts::FJsEnv>();
    JsEnv = MakeShared<puerts::FJsEnv>(
        std::make_unique<puerts::DefaultJSModuleLoader>(TEXT("JavaScript")),
        std::make_shared<puerts::FDefaultLogger>(), 8889);
    JsEnv->WaitDebugger();
    TArray<TPair<FString, UObject*>> Arguments;
    Arguments.Add(TPair<FString, UObject*>(TEXT("GameMode"), this));
    JsEnv->Start("LearPuerTsQuickStart", Arguments);
}
