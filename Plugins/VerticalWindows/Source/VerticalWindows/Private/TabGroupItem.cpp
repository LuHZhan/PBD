#include "TabGroupItem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UTabGroupItem::NativeConstruct()
{
	Super::NativeConstruct();

	if (RootButton)
	{
		RootButton->OnClicked.AddDynamic(this, &UTabGroupItem::HandleButtonClicked);
	}
}

void UTabGroupItem::SetGroupData(const FString& InGroupId, const FString& InGroupName, FLinearColor InColor)
{
	GroupId = InGroupId;
	GroupName = InGroupName;
	GroupColor = InColor;
	bIsCreateNewButton = false;

	OnDataUpdated();
}

void UTabGroupItem::SetAsCreateNewButton()
{
	GroupId = TEXT("__NEW_GROUP__");
	GroupName = TEXT("Create Group");
	GroupColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
	bIsCreateNewButton = true;

	OnDataUpdated();
}

void UTabGroupItem::OnDataUpdated_Implementation()
{
	// 更新颜色图标
	if (ColorIcon)
	{
		ColorIcon->SetColorAndOpacity(GroupColor);
	}

	// 更新群组名称
	if (GroupNameText)
	{
		GroupNameText->SetText(FText::FromString(GroupName));
	}
}

void UTabGroupItem::HandleButtonClicked()
{
	OnClicked.Broadcast(GroupId);
}
