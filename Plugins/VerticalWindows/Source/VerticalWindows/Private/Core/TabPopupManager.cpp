#include "TabPopupManager.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/CoreStyle.h"

// 初始化静态成员
TMap<UUserWidget*, TWeakPtr<SWindow>> UTabPopupManager::ActivePopups;

UUserWidget* UTabPopupManager::ShowPopup(
	TSubclassOf<UUserWidget> WidgetClass,
	FVector2D ScreenPosition,
	FVector2D Size,
	bool bCloseOnClickOutside)
{
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TabPopupManager] WidgetClass is null"));
		return nullptr;
	}

	// 获取编辑器 World
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[TabPopupManager] Cannot get editor World"));
		return nullptr;
	}

	// 创建 UMG Widget
	UUserWidget* Widget = CreateWidget<UUserWidget>(World, WidgetClass);
	if (!Widget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TabPopupManager] Failed to create widget"));
		return nullptr;
	}

	// 获取 Slate Widget
	TSharedRef<SWidget> SlateWidget = Widget->TakeWidget();

	// 构建窗口内容（无边框）
	TSharedRef<SWidget> WindowContent = 
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
		.Padding(0)
		[
			SlateWidget
		];

	// 确定窗口大小
	FVector2D WindowSize = Size;
	if (Size.IsZero())
	{
		// 自动大小 - 使用 Widget 的期望大小
		WindowSize = FVector2D(300, 200); // 默认回退值
		FVector2D DesiredSize = Widget->GetDesiredSize();
		if (DesiredSize.X > 0 && DesiredSize.Y > 0)
		{
			WindowSize = DesiredSize;
		}
	}

	// 创建 SWindow
	TSharedRef<SWindow> PopupWindow = SNew(SWindow)
		.Type(EWindowType::Menu)
		.IsPopupWindow(true)
		.SizingRule(Size.IsZero() ? ESizingRule::Autosized : ESizingRule::FixedSize)
		.ClientSize(WindowSize)
		.ScreenPosition(ScreenPosition)
		.AutoCenter(EAutoCenter::None)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.HasCloseButton(false)
		.CreateTitleBar(false)
		.IsTopmostWindow(true)
		.FocusWhenFirstShown(true)
		.ActivationPolicy(bCloseOnClickOutside ? EWindowActivationPolicy::Always : EWindowActivationPolicy::FirstShown)
		[
			WindowContent
		];

	// 添加窗口到 Slate 应用
	FSlateApplication::Get().AddWindow(PopupWindow);

	// 追踪弹窗
	ActivePopups.Add(Widget, PopupWindow);

	// 处理窗口关闭事件
	PopupWindow->SetOnWindowClosed(FOnWindowClosed::CreateLambda(
		[Widget](const TSharedRef<SWindow>&)
		{
			ActivePopups.Remove(Widget);
			if (Widget && Widget->IsValidLowLevel())
			{
				Widget->RemoveFromParent();
			}
			UE_LOG(LogTemp, Log, TEXT("[TabPopupManager] Popup window closed"));
		}
	));

	UE_LOG(LogTemp, Log, TEXT("[TabPopupManager] Popup shown at (%.1f, %.1f), Size: (%.1f, %.1f)"),
		ScreenPosition.X, ScreenPosition.Y, WindowSize.X, WindowSize.Y);

	return Widget;
}

UUserWidget* UTabPopupManager::ShowPopupAtCursor(
	TSubclassOf<UUserWidget> WidgetClass,
	FVector2D Size,
	bool bCloseOnClickOutside)
{
	FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
	return ShowPopup(WidgetClass, CursorPos, Size, bCloseOnClickOutside);
}

void UTabPopupManager::ClosePopup(UUserWidget* Widget)
{
	if (!Widget)
	{
		return;
	}

	TWeakPtr<SWindow>* WindowPtr = ActivePopups.Find(Widget);
	if (WindowPtr && WindowPtr->IsValid())
	{
		TSharedPtr<SWindow> Window = WindowPtr->Pin();
		if (Window.IsValid())
		{
			FSlateApplication::Get().RequestDestroyWindow(Window.ToSharedRef());
			UE_LOG(LogTemp, Log, TEXT("[TabPopupManager] Closing popup window"));
		}
	}
	
	ActivePopups.Remove(Widget);
}

void UTabPopupManager::CloseAllPopups()
{
	UE_LOG(LogTemp, Log, TEXT("[TabPopupManager] Closing all popups (%d active)"), ActivePopups.Num());
	
	for (auto& Pair : ActivePopups)
	{
		if (Pair.Value.IsValid())
		{
			TSharedPtr<SWindow> Window = Pair.Value.Pin();
			if (Window.IsValid())
			{
				FSlateApplication::Get().RequestDestroyWindow(Window.ToSharedRef());
			}
		}
	}
	
	ActivePopups.Empty();
}

bool UTabPopupManager::IsPopupActive(UUserWidget* Widget)
{
	if (!Widget)
	{
		return false;
	}
	
	TWeakPtr<SWindow>* WindowPtr = ActivePopups.Find(Widget);
	return WindowPtr && WindowPtr->IsValid();
}
