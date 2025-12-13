#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "CommonLuBtn.generated.h"

// 按钮状态枚举
UENUM(BlueprintType)
enum class ELuBtnState : uint8
{
	Normal,
	Hovered,
	Selected
};

// 委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLuBtnHovered_Post);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLuBtnUnhovered_Post);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLuBtnClicked_Post);

/**
 * LuBtn - 通用按钮组件
 * 
 * 特性:
 * - 三态切换（Normal/Hovered/Selected）
 * - 完全可自定义外观（颜色、大小、文本）
 * - 蓝图重载函数 (_Post 后缀)
 * - 外部事件委托 (_Post 后缀)
 */
UCLASS(BlueprintType, Blueprintable)
class VERTICALWINDOWS_API ULuBtn : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============ UI 组件绑定 ============

	/** 背景切换器 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UWidgetSwitcher* BGSwitcher;

	/** 正常状态背景 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* NormalBG;

	/** 悬停状态背景 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* HoveredBG;

	/** 选中状态背景 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* SelectedBG;

	/** 文本显示 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	USizeBox* ItemSizeBox;

	/** 点击按钮（处理点击事件）*/
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* ClickButton;

	// ============ 可编辑属性 - 外观 ============

	/** 正常状态颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LuBtn|Appearance")
	FLinearColor NormalColor = FLinearColor(0.039546f, 0.039546f, 0.039546f, 1.0f);

	/** 悬停状态颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LuBtn|Appearance")
	FLinearColor HoveredColor = FLinearColor(0.3f, 0.5f, 0.8f, 1.0f);

	/** 选中状态颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LuBtn|Appearance")
	FLinearColor SelectedColor = FLinearColor(0.2f, 0.4f, 0.7f, 1.0f);

	// ============ 可编辑属性 - 尺寸 ============

	/** 按钮大小 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LuBtn|Size")
	FVector2D CurItemSize = FVector2D(200.0f, 40.0f);

	// ============ 可编辑属性 - 文本 ============

	/** 按钮文本 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LuBtn|Text")
	FText ButtonText = FText::FromString("Button");

	/** 字体大小 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LuBtn|Text")
	int32 FontSize = 14;

	/** 文本颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LuBtn|Text")
	FLinearColor TextColor = FLinearColor::White;

	/** 文本边距 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LuBtn|Text")
	FMargin TextPadding = FMargin(10.0f, 5.0f);

	// ============ 状态 ============

	/** 当前状态 */
	UPROPERTY(BlueprintReadOnly, Category = "LuBtn")
	ELuBtnState CurrentState = ELuBtnState::Normal;

	/** 是否选中 */
	UPROPERTY(BlueprintReadOnly, Category = "LuBtn")
	bool bIsSelected = false;

	// ============ 公共方法 ============

	/** 设置选中状态 */
	UFUNCTION(BlueprintCallable, Category = "LuBtn")
	void SetIsSelected(bool bSelected);

	/** 设置按钮状态 */
	UFUNCTION(BlueprintCallable, Category = "LuBtn")
	void SetButtonState(ELuBtnState NewState);

	/** 设置文本 */
	UFUNCTION(BlueprintCallable, Category = "LuBtn")
	void SetButtonText(const FText& NewText);

	/** 应用样式到组件 */
	UFUNCTION(BlueprintCallable, Category = "LuBtn")
	void ApplyStyle();

	// ============ 蓝图重载函数 (供蓝图扩展逻辑) ============

	/** 悬停时触发（蓝图可重载） */
	UFUNCTION(BlueprintNativeEvent, Category = "LuBtn|Events")
	void OnHovered_Post();

	/** 离开悬停时触发（蓝图可重载） */
	UFUNCTION(BlueprintNativeEvent, Category = "LuBtn|Events")
	void OnUnhovered_Post();

	/** 点击时触发（蓝图可重载） */
	UFUNCTION(BlueprintNativeEvent, Category = "LuBtn|Events")
	void OnClicked_Post();

	// ============ 委托 (供外部绑定) ============

	/** 悬停委托 */
	UPROPERTY(BlueprintAssignable, Category = "LuBtn|Delegates")
	FOnLuBtnHovered_Post OnHoveredDelegate_Post;

	/** 离开悬停委托 */
	UPROPERTY(BlueprintAssignable, Category = "LuBtn|Delegates")
	FOnLuBtnUnhovered_Post OnUnhoveredDelegate_Post;

	/** 点击委托 */
	UPROPERTY(BlueprintAssignable, Category = "LuBtn|Delegates")
	FOnLuBtnClicked_Post OnClickedDelegate_Post;

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

	// ============ 鼠标事件重写 ============

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	// ============ 按钮点击处理 ============
	
	/** UButton 点击事件处理 */
	UFUNCTION()
	void HandleButtonClicked();

private:
	/** 更新背景颜色 */
	void UpdateBackgroundColors();

	/** 更新文本样式 */
	void UpdateTextStyle();

	/** 更新尺寸 */
	void UpdateSize();

	/** 切换到指定状态 */
	void SwitchToState(ELuBtnState NewState);
};
