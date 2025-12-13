#include "CommonLuBtn.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void ULuBtn::NativeConstruct()
{
	Super::NativeConstruct();

	// 应用初始样式
	ApplyStyle();

	// 设置初始状态为 Normal
	SetButtonState(ELuBtnState::Normal);
	
	// 🔧 绑定 UButton 的点击事件
	if (ClickButton)
	{
		ClickButton->OnClicked.AddDynamic(this, &ULuBtn::HandleButtonClicked);
	}
}

void ULuBtn::NativePreConstruct()
{
	Super::NativePreConstruct();
	// 在设计时也应用样式（实时预览）
	ApplyStyle();
}

// ============ 鼠标事件处理 ============

void ULuBtn::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	// 如果未选中，切换到悬停状态
	if (!bIsSelected)
	{
		SetButtonState(ELuBtnState::Hovered);
	}

	// 触发蓝图重载函数
	OnHovered_Post();

	// 触发委托
	OnHoveredDelegate_Post.Broadcast();
}

void ULuBtn::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	// 如果未选中，切换回正常状态
	if (!bIsSelected)
	{
		SetButtonState(ELuBtnState::Normal);
	}

	// 触发蓝图重载函数
	OnUnhovered_Post();

	// 触发委托
	OnUnhoveredDelegate_Post.Broadcast();
}

// ============ 按钮点击处理 ============

void ULuBtn::HandleButtonClicked()
{
	// 切换到选中状态
	SetIsSelected(true);
	
	// 触发蓝图重载函数
	OnClicked_Post();

	// 触发委托
	OnClickedDelegate_Post.Broadcast();
}

// ============ 公共方法 ============

void ULuBtn::SetIsSelected(bool bSelected)
{
	bIsSelected = bSelected;

	if (bSelected)
	{
		SetButtonState(ELuBtnState::Selected);
	}
	else
	{
		SetButtonState(ELuBtnState::Normal);
	}
}

void ULuBtn::SetButtonState(ELuBtnState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;
	SwitchToState(NewState);
}

void ULuBtn::SetButtonText(const FText& NewText)
{
	ButtonText = NewText;

	if (ItemText)
	{
		ItemText->SetText(ButtonText);
	}
}

void ULuBtn::ApplyStyle()
{
	UpdateBackgroundColors();
	UpdateTextStyle();
	UpdateSize();
}

// ============ 私有方法 ============

void ULuBtn::UpdateBackgroundColors()
{
	if (NormalBG)
	{
		NormalBG->SetColorAndOpacity(NormalColor);
	}

	if (HoveredBG)
	{
		HoveredBG->SetColorAndOpacity(HoveredColor);
	}

	if (SelectedBG)
	{
		SelectedBG->SetColorAndOpacity(SelectedColor);
	}
}

void ULuBtn::UpdateTextStyle()
{
	if (ItemText)
	{
		// 设置文本内容
		ItemText->SetText(ButtonText);

		// 设置文本颜色
		ItemText->SetColorAndOpacity(FSlateColor(TextColor));

		// 设置字体大小
		FSlateFontInfo FontInfo = ItemText->GetFont();
		FontInfo.Size = FontSize;
		ItemText->SetFont(FontInfo);

		// 设置文本边距（通过父容器的 Padding）
		// 注意：这需要在蓝图中将 ItemText 放在一个容器内
	}
}

void ULuBtn::UpdateSize()
{
	ItemSizeBox->SetWidthOverride(CurItemSize.X);
	ItemSizeBox->SetHeightOverride(CurItemSize.Y);
}

void ULuBtn::SwitchToState(ELuBtnState NewState)
{
	if (!BGSwitcher)
	{
		return;
	}

	switch (NewState)
	{
	case ELuBtnState::Normal:
		BGSwitcher->SetActiveWidgetIndex(0); // NormalBG
		break;

	case ELuBtnState::Hovered:
		BGSwitcher->SetActiveWidgetIndex(1); // HoveredBG
		break;

	case ELuBtnState::Selected:
		BGSwitcher->SetActiveWidgetIndex(2); // SelectedBG
		break;
	}
}

// ============ 蓝图重载函数默认实现 ============

void ULuBtn::OnHovered_Post_Implementation()
{
	// 蓝图可以重载此函数添加自定义逻辑
	// 默认实现为空
}

void ULuBtn::OnUnhovered_Post_Implementation()
{
	// 蓝图可以重载此函数添加自定义逻辑
	// 默认实现为空
}

void ULuBtn::OnClicked_Post_Implementation()
{
	// 蓝图可以重载此函数添加自定义逻辑
	// 默认实现为空
}
