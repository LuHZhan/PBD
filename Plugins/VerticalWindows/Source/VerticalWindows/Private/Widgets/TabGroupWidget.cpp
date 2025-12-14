#include "TabGroupWidget.h"
#include "TabItemWidget.h"
#include "TabManager.h"
#include "Components/Button.h"
#include "Components/ExpandableArea.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

// ============ 生命周期 ============

void UTabGroupWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 🔧 绑定 HeaderButton 点击事件
	if (HeaderButton)
	{
		HeaderButton->OnClicked.AddDynamic(this, &UTabGroupWidget::HandleHeaderButtonClicked);
	}
	
	// 初始化颜色和文本
	UpdateExpandableAreaColor();
	UpdateGroupNameText();
}

// ============ Manager 和数据 ============

void UTabGroupWidget::SetTabManager(UTabManager* Manager)
{
	TabManager = Manager;

	// 更新所有子项
	for (UTabItemWidget* ItemWidget : ChildItemWidgets)
	{
		if (ItemWidget)
		{
			ItemWidget->SetTabManager(Manager);
		}
	}
}

void UTabGroupWidget::SetGroupData(const FTabGroupInfo& InData)
{
	GroupData = InData;
	OnGroupDataUpdated();
	SetExpanded(InData.bExpanded);
	
	// 🔧 更新显示
	UpdateGroupNameText();
	UpdateExpandableAreaColor();
}

void UTabGroupWidget::AddTab(const FEditorTabInfo& TabInfo)
{
	GroupData.Tabs.Add(TabInfo);
	OnTabAdded(TabInfo);
}

void UTabGroupWidget::AddChildWidget(UUserWidget* Widget)
{
	if (ItemContainer && Widget)
	{
		ItemContainer->AddChildToVerticalBox(Widget);

		// 如果是 TabItemWidget，跟踪并设置
		if (UTabItemWidget* TabItem = Cast<UTabItemWidget>(Widget))
		{
			ChildItemWidgets.Add(TabItem);

			// 设置 Manager
			if (TabManager.IsValid())
			{
				TabItem->SetTabManager(TabManager.Get());
			}

			// 绑定右键事件（转发给 UI 层）
			TabItem->OnRightClicked.AddDynamic(this, &UTabGroupWidget::HandleChildItemRightClicked);
		}
	}
}

void UTabGroupWidget::ClearTabs()
{
	GroupData.Tabs.Empty();
	ChildItemWidgets.Empty();

	if (ItemContainer)
	{
		ItemContainer->ClearChildren();
	}
	OnTabsCleared();
}

void UTabGroupWidget::SetExpanded(bool bExpanded)
{
	GroupData.bExpanded = bExpanded;
	OnExpansionStateChanged(bExpanded);

	if (OnToggle.IsBound())
	{
		OnToggle.Broadcast(GroupData, bExpanded);
	}

	// 🔧 同步 GroupExpandableArea 状态
	if (GroupExpandableArea)
	{
		GroupExpandableArea->SetIsExpanded(bExpanded);
	}
	
	if (ItemContainer)
	{
		ItemContainer->SetVisibility(bExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	
	// 🔧 更新背景颜色
	UpdateExpandableAreaColor();
}

void UTabGroupWidget::OnGroupDataUpdated_Implementation()
{
	// 默认实现 - 蓝图可重写
}

void UTabGroupWidget::HandleHeaderClicked()
{
	SetExpanded(!GroupData.bExpanded);
}

void UTabGroupWidget::HandleChildItemRightClicked(const FEditorTabInfo& TabInfo, FVector2D ScreenPosition)
{
	// 转发到 UI 层
	OnItemRightClicked.Broadcast(GroupData, TabInfo, ScreenPosition);
}

// ============ 🆕 新增功能实现 ============

void UTabGroupWidget::HandleHeaderButtonClicked()
{
	// 切换展开状态
	SetExpanded(!GroupData.bExpanded);
	
	// 更新背景颜色
	UpdateExpandableAreaColor();
}

void UTabGroupWidget::UpdateExpandableAreaColor()
{
	if (!ExpandableAreaBG)
	{
		return;
	}
	
	// 颜色方案：
	// 展开时：使用类型 icon 颜色（鲜艳）
	// 收起时：暗淡版本（Color * 0.4 + 深灰 0.6）
	
	FLinearColor TargetColor;
	
	if (GroupData.bExpanded)
	{
		// 展开：使用完整的类型颜色
		TargetColor = GroupData.Color;
	}
	else
	{
		// 收起：混合 40% 类型颜色 + 60% 深灰
		FLinearColor DarkGray = FLinearColor(0.08f, 0.08f, 0.08f, 1.0f);
		TargetColor = GroupData.Color * 0.4f + DarkGray * 0.6f;
		TargetColor.A = 1.0f;  // 保持不透明
	}
	
	// 应用颜色
	ExpandableAreaBG->SetColorAndOpacity(TargetColor);
	
	UE_LOG(LogTemp, Log, TEXT("[TabGroupWidget] Background color updated: Expanded=%d, Color=(%.2f, %.2f, %.2f)"),
		GroupData.bExpanded, TargetColor.R, TargetColor.G, TargetColor.B);
}

void UTabGroupWidget::UpdateGroupNameText()
{
	if (!GroupNameText)
	{
		return;
	}
	
	// 更新为类型名称
	FText GroupName = FText::FromString(GroupData.GroupName);
	GroupNameText->SetText(GroupName);
	
	UE_LOG(LogTemp, Log, TEXT("[TabGroupWidget] Group name updated: %s"), *GroupData.GroupName);
}
