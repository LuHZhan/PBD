// Copyright Epic Games, Inc. All Rights Reserved.

#include "VerticalWindows.h"

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "VerticalWindowsStyle.h"
#include "VerticalWindowsCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName VerticalWindowsTabName("VerticalWindows");

#define LOCTEXT_NAMESPACE "FVerticalWindowsModule"

void FVerticalWindowsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FVerticalWindowsStyle::Initialize();
	FVerticalWindowsStyle::ReloadTextures();

	FVerticalWindowsCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FVerticalWindowsCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FVerticalWindowsModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FVerticalWindowsModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(VerticalWindowsTabName, FOnSpawnTab::CreateRaw(this, &FVerticalWindowsModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FVerticalWindowsTabTitle", "VerticalWindows"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FVerticalWindowsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FVerticalWindowsStyle::Shutdown();

	FVerticalWindowsCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(VerticalWindowsTabName);
	
	JsEnv.Reset();
	UE_LOG(LogTemp, Log, TEXT("[VerticalWindows] TypeScript shutdown"));
}

TSharedRef<SDockTab> FVerticalWindowsModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	// 创建 JS 环境
	JsEnv = MakeShared<puerts::FJsEnv>(
		std::make_unique<puerts::DefaultJSModuleLoader>(TEXT("JavaScript")),
		std::make_shared<puerts::FDefaultLogger>(),
		-1
	);
    
	// 启动 TypeScript
	JsEnv->Start("Editor/Tab/Main");
    
	UE_LOG(LogTemp, Log, TEXT("[VerticalWindows] TypeScript initialized"));

	
	//加载UMG的BP
	UEditorUtilityWidgetBlueprint* UMGBP = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr,
	TEXT("/VerticalWindows/Editor/EDU_OpenedEditor.EDU_OpenedEditor"));

	if (UMGBP)
	{
		UEditorUtilitySubsystem* Subsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		Subsystem->SpawnAndRegisterTab(UMGBP);
	}

	//仿照 UEditorUtilityWidgetBlueprint::CreateUtilityWidget() 再实现一遍
	TSharedRef<SWidget> TabWidget = SNullWidget::NullWidget;
	{
		UEditorUtilityWidget* CreatedUMGWidget = nullptr;

		UClass* BlueprintClass = UMGBP->GeneratedClass;
		TSubclassOf<UEditorUtilityWidget> WidgetClass = BlueprintClass;
		UWorld* World = GEditor->GetEditorWorldContext().World();
		if (World)
		{
			if (CreatedUMGWidget)
			{
				CreatedUMGWidget->Rename(nullptr, GetTransientPackage());
			}
			CreatedUMGWidget = CreateWidget<UEditorUtilityWidget>(World, WidgetClass);
		}
		if (CreatedUMGWidget)
		{
			TabWidget = SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					CreatedUMGWidget->TakeWidget()
				];
		}
	}

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			TabWidget
		];
}

void FVerticalWindowsModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(VerticalWindowsTabName);
}

void FVerticalWindowsModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);
	
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FVerticalWindowsCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FVerticalWindowsCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVerticalWindowsModule, VerticalWindows)