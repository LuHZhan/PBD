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
 *  控制空间 (World / Parent):
 *  +----------+----------------------------------------------+
 *  | World    | 世界空间控制 - 骨骼相对世界坐标系驱动              
 *  | Parent   | 父骨骼空间控制 - 骨骼相对父骨骼驱动                
 *  +----------+----------------------------------------------+
 *  | Str      | Strength 弹簧刚度 (0-1 -> 0-1000)            
 *  | Damp     | Damping 阻尼系数 (0-1 -> 0-100)              
 *  +----------+----------------------------------------------+
 *
 *  右侧按钮 (P/G/S/M/B):
 *  +-----+--------------+------------------------------------+
 *  | [P] | Physics      | 启用/禁用物理控制                  
 *  | [G] | Gravity      | 重力开关 (0/1)                     
 *  | [S] | Simulate     | Simulated/Kinematic 模式切换       
 *  | [M] | Multiplier   | 强度乘数 (0.5->1->1.5->2 循环)     
 *  | [B] | Blend        | 动画<->物理混合 (0=动画, 1=物理)   
 *  +-----+--------------+------------------------------------+
 *
 *  底部控件 (T + 滑块 / R/I/W/S):
 *  +-----+--------------+------------------------------------+
 *  | [T] | Target       | 使用骨骼动画作为驱动目标           
 *  | --- | Slider       | 全局强度乘数滑块                   
 *  +-----+--------------+------------------------------------+
 *  | [R] | Reset        | 重置到默认参数                     
 *  | [I] | Initialize   | 重新初始化物理控制                 
 *  | [W] | World        | 强制世界空间覆盖                   
 *  | [S] | Skeletal     | 使用骨骼动画目标                   
 *  +-----+--------------+------------------------------------+
 *
 *  物理效果组合:
 *  +----------+----------+-----------------------------------+
 *  | Strength | Damping  | Effect                            
 *  +----------+----------+-----------------------------------+
 *  | High     | Low      | 快速响应, 可能振荡                
 *  | High     | High     | 稳定缓慢, 像在焦油中              
 *  | Low      | Low      | 松散飘忽                          
 *  | Low      | High     | 几乎不动                          
 *  +----------+----------+-----------------------------------+
 *
 *  代码结构:
 *  SVerticalBox (根)
 *  |
 *  +-- Slot[0]: SHorizontalBox
 *  |   +-- World 组 (SVerticalBox)
 *  |   +-- Parent 组 (SVerticalBox)
 *  |   +-- 右侧按钮列 (P/G/S/M/B)
 *  |
 *  +-- Slot[1]: [T] + 水平滑块 (全局乘数)
 *  |
 *  +-- Slot[2]: SHorizontalBox (R/I/W/S)
 *
 *  参考: GDC2023 "New Character Physics in UE5: Can You Pet the Dog?"
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
	
	FSliderStyle CustomSliderThumbStyle;
	
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
	void InitializeSliderImageBrush();
};
