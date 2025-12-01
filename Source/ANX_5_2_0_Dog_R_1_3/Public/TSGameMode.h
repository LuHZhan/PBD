// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsEnv.h"
#include "GameFramework/GameMode.h"
#include "TSGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ANX_5_2_0_DOG_R_1_3_API ATSGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void ExecuteScript(FString ScriptPath, TMap<FString, UObject*> InArgs, bool bWithDebug);

	UFUNCTION(BlueprintCallable)
	void OnStart();
	
	TSharedPtr<puerts::FJsEnv> JsEnv;
};
