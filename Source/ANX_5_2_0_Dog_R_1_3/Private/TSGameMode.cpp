// Fill out your copyright notice in the Description page of Project Settings.

#include "TSGameMode.h"

void ATSGameMode::ExecuteScript(FString ScriptPath,
                                TMap<FString, UObject*> InArgs,
                                bool bWithDebug)
{
    if (JsEnv)
    {
        JsEnv.Reset();
    }

    if (bWithDebug)
    {
        JsEnv = MakeShared<puerts::FJsEnv>(
            std::make_unique<puerts::DefaultJSModuleLoader>(TEXT("JavaScript")),
            std::make_shared<puerts::FDefaultLogger>(), 8889);
        JsEnv->WaitDebugger();
    }
    else
    {
        JsEnv = MakeShared<puerts::FJsEnv>();
    }

    if (JsEnv)
    {
        TArray<TPair<FString, UObject*>> RealArgs;
        for (const TPair<FString, UObject*>& Kvp : InArgs)
        {
            if (Kvp.Value != nullptr)
            {
                const TPair<FString, UObject*> Item =
                    TPair<FString, UObject*>(Kvp.Key, Kvp.Value);
                RealArgs.Add(Item);
            }
        }
        FString ScriptBaseName = FPaths::GetBaseFilename(ScriptPath);
        JsEnv->Start(ScriptBaseName, RealArgs);
    }
}

void ATSGameMode::OnStart()
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
