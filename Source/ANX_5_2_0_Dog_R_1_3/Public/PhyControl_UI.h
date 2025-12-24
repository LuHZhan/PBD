// PhyControl_UI.h
// Physics Control 用户界面 - 事件驱动版本

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PhyControlTypes.h"
#include "PhyControl_UI.generated.h"

/**
 *  Physics Control UI
 *
 *  功能布局:
 *  ┌──────────────────────────────────────────────────────┐
 *  │                                                      │
 *  │   ●World          ●Parent                      [P]   │
 *  │                                                [G]   │
 *  │   ││  ││          ││  ││                       [S]   │
 *  │   ││  ││          ││  ││                       [M]   │
 *  │   Str Damp        Str Damp                     [B]   │
 *  │                                                      │
 *  │   [T] ════════════════════════════════════════════   │
 *  │                                                      │
 *  │   [R] [I] [W] [S]                                    │
 *  │                                                      │
 *  └──────────────────────────────────────────────────────┘
 *
 *  代码结构:
 *  SVerticalBox (根)
 *  │
 *  ├── Slot[0]: SHorizontalBox
 *  │   ├── World 组 (SVerticalBox)
 *  │   ├── Parent 组 (SVerticalBox)
 *  │   └── 右侧按钮列 (P/G/S/M/B)
 *  │
 *  ├── Slot[1]: 水平滑块 (全局乘数)
 *  │
 *  └── Slot[2]: SHorizontalBox (R/I/W/S)
 *
 *  性能优化:
 *  - 使用事件驱动而非轮询 (_Lambda)
 *  - 保存控件引用，点击时直接调用 SetXXX
 */


UCLASS()
class UPhyControl_UI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ==================== 数据绑定 ====================

	UPROPERTY(BlueprintReadOnly, Category = "Physics Control")
	FPhyControlUIData ControlData;

	UPROPERTY(BlueprintAssignable, Category = "Physics Control")
	FOnPhyControlDataChanged OnDataChanged;

	UPROPERTY(BlueprintAssignable, Category = "Physics Control")
	FOnPhyControlValueChanged OnValueChanged;

	UPROPERTY(BlueprintAssignable, Category = "Physics Control")
	FOnPhyControlBoolChanged OnBoolChanged;

	// ==================== 公共方法 ====================

	UFUNCTION(BlueprintCallable, Category = "Physics Control")
	void SetControlData(const FPhyControlUIData& InData);

	UFUNCTION(BlueprintCallable, Category = "Physics Control")
	void SetTargetLimb(FName LimbName);

	UFUNCTION(BlueprintCallable, Category = "Physics Control")
	void RefreshUI();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void NativePreConstruct() override;

private:
	// ==================== 控件引用 ====================

	TSharedPtr<SWidget> RootWidget;

	// 指示器图标
	TSharedPtr<SImage> WorldIndicator;
	TSharedPtr<SImage> ParentIndicator;

	// 右侧按钮指示器 (P/G/S/M/B)
	TSharedPtr<SBorder> ButtonP_Indicator;
	TSharedPtr<SBorder> ButtonG_Indicator;
	TSharedPtr<SBorder> ButtonS_Indicator;
	TSharedPtr<SBorder> ButtonM_Indicator;
	TSharedPtr<SBorder> ButtonB_Indicator;

	// T 按钮指示器
	TSharedPtr<SBorder> ButtonT_Indicator;

	// 底部按钮
	TSharedPtr<SButton> ButtonR;
	TSharedPtr<SButton> ButtonI;
	TSharedPtr<SButton> ButtonW;
	TSharedPtr<SButton> ButtonS_Bottom;
	

	// ==================== 面板创建 ====================

	TSharedRef<SWidget> CreateControlSpacePanel(
		const FString& Title,
		FPhyControlSpaceData* SpaceData,
		TSharedPtr<SImage>& OutIndicator,
		TFunction<void()> OnToggled
	);

	TSharedRef<SWidget> CreateRightButtonColumn();

	// ==================== 辅助控件创建 ====================

	// 圆形指示器按钮 (World/Parent 标题前的圆点)
	TSharedRef<SWidget> CreateIndicatorButton(
		bool* EnabledPtr,
		TSharedPtr<SImage>& OutIndicator,
		TFunction<void()> OnClicked
	);

	// 右侧/T 圆形按钮
	TSharedRef<SWidget> CreateCircleButton(
		const FString& Label,
		bool* EnabledPtr,
		TSharedPtr<SBorder>& OutIndicator,
		TFunction<void()> OnClicked
	);

	// 底部橙色按钮 (R/I/W/S)
	TSharedRef<SWidget> CreateBottomButton(
		const FString& Label,
		TSharedPtr<SButton>& OutButton,
		TFunction<void()> OnClicked
	);

	// 自定义垂直滑块 (带横向长方形滑块)
	TSharedRef<SWidget> CreateCustomVerticalSlider(
		const FString& Label,
		float* ValuePtr,
		TFunction<void(float)> OnChanged
	);

	// 自定义水平滑块 (带横向长方形滑块)
	TSharedRef<SWidget> CreateCustomHorizontalSlider(
		float* ValuePtr,
		float MinValue,
		float MaxValue,
		TFunction<void(float)> OnChanged
	);
	
	FSliderStyle CustomSliderStyle;
	
	// ==================== UI 更新 ====================

	void UpdateIndicator(TSharedPtr<SImage>& Indicator, bool bEnabled);
	void UpdateCircleButton(TSharedPtr<SBorder>& Indicator, bool bEnabled);

	// ==================== 蓝图可重载 ====================

public:
	/** 获取指示器 Brush,蓝图可重载自定义外观 */
	UFUNCTION(BlueprintNativeEvent, Category = "Physics Control|Style")
	void UpdateIndicatorBrush(bool bEnabled) const;

	// ==================== 样式缓存 ====================

	// 指示器 Brush 缓存（用于 SetImage）
	UPROPERTY(BlueprintReadWrite, Category = "Physics Control")
	FSlateBrush EnabledBrush;
	
	UPROPERTY(BlueprintReadWrite, Category = "Physics Control")
	FSlateBrush DisabledBrush;

	// ==================== 回调 ====================

	void OnWorldToggle();
	void OnParentToggle();
	void NotifyDataChanged();
	void OnResetRequested();
	void OnInitializeRequested();
};
