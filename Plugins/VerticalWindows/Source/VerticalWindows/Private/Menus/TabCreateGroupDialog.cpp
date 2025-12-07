#include "TabCreateGroupDialog.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/Image.h"

void UTabCreateGroupDialog::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化预设颜色
	InitializePresetColors();

	// 绑定按钮事件
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UTabCreateGroupDialog::HandleConfirmClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UTabCreateGroupDialog::HandleCancelClicked);
	}

	// 绑定颜色按钮
	BindColorButtons();

	// 设置默认颜色
	if (PresetColors.Num() > 0)
	{
		SelectColor(PresetColors[0]);
	}
	else
	{
		SelectColor(FLinearColor(0.29f, 0.56f, 0.85f, 1.0f)); // 默认蓝色
	}
}

void UTabCreateGroupDialog::InitializePresetColors()
{
	if (PresetColors.Num() == 0)
	{
		// 8种预设颜色（类似浏览器标签组颜色）
		PresetColors.Add(FLinearColor(0.91f, 0.30f, 0.24f, 1.0f));  // 红色
		PresetColors.Add(FLinearColor(0.90f, 0.49f, 0.13f, 1.0f));  // 橙色
		PresetColors.Add(FLinearColor(0.95f, 0.77f, 0.06f, 1.0f));  // 黄色
		PresetColors.Add(FLinearColor(0.15f, 0.68f, 0.38f, 1.0f));  // 绿色
		PresetColors.Add(FLinearColor(0.10f, 0.74f, 0.61f, 1.0f));  // 青色
		PresetColors.Add(FLinearColor(0.29f, 0.56f, 0.85f, 1.0f));  // 蓝色
		PresetColors.Add(FLinearColor(0.56f, 0.27f, 0.68f, 1.0f));  // 紫色
		PresetColors.Add(FLinearColor(0.91f, 0.30f, 0.54f, 1.0f));  // 粉色
	}
}

void UTabCreateGroupDialog::BindColorButtons()
{
	if (ColorBtn_Red)
	{
		ColorBtn_Red->OnClicked.AddDynamic(this, &UTabCreateGroupDialog::HandleColorRedClicked);
	}
	if (ColorBtn_Orange)
	{
		ColorBtn_Orange->OnClicked.AddDynamic(this, &UTabCreateGroupDialog::HandleColorOrangeClicked);
	}
	if (ColorBtn_Yellow)
	{
		ColorBtn_Yellow->OnClicked.AddDynamic(this, &UTabCreateGroupDialog::HandleColorYellowClicked);
	}
	if (ColorBtn_Green)
	{
		ColorBtn_Green->OnClicked.AddDynamic(this, &UTabCreateGroupDialog::HandleColorGreenClicked);
	}
	if (ColorBtn_Cyan)
	{
		ColorBtn_Cyan->OnClicked.AddDynamic(this, &UTabCreateGroupDialog::HandleColorCyanClicked);
	}
	if (ColorBtn_Blue)
	{
		ColorBtn_Blue->OnClicked.AddDynamic(this, &UTabCreateGroupDialog::HandleColorBlueClicked);
	}
	if (ColorBtn_Purple)
	{
		ColorBtn_Purple->OnClicked.AddDynamic(this, &UTabCreateGroupDialog::HandleColorPurpleClicked);
	}
	if (ColorBtn_Pink)
	{
		ColorBtn_Pink->OnClicked.AddDynamic(this, &UTabCreateGroupDialog::HandleColorPinkClicked);
	}
}

void UTabCreateGroupDialog::ShowDialog()
{
	// 清空输入
	if (NameInputBox)
	{
		NameInputBox->SetText(FText::GetEmpty());
	}

	// 重置颜色
	if (PresetColors.Num() > 0)
	{
		SelectColor(PresetColors[0]);
	}

	SetVisibility(ESlateVisibility::Visible);
	OnDialogOpened();
}

void UTabCreateGroupDialog::CloseDialog()
{
	SetVisibility(ESlateVisibility::Collapsed);
	OnDialogClosed();
}

void UTabCreateGroupDialog::SelectColor(FLinearColor NewColor)
{
	SelectedColor = NewColor;
	UpdateColorPreview();
	OnColorChanged(NewColor);
}

void UTabCreateGroupDialog::SelectPresetColor(int32 ColorIndex)
{
	if (PresetColors.IsValidIndex(ColorIndex))
	{
		SelectColor(PresetColors[ColorIndex]);
	}
}

void UTabCreateGroupDialog::UpdateColorPreview()
{
	if (ColorPreview)
	{
		ColorPreview->SetColorAndOpacity(SelectedColor);
	}
}

FString UTabCreateGroupDialog::GetGroupName() const
{
	if (NameInputBox)
	{
		return NameInputBox->GetText().ToString();
	}
	return FString();
}

bool UTabCreateGroupDialog::IsInputValid() const
{
	FString Name = GetGroupName();
	return !Name.IsEmpty() && Name.Len() >= 1;
}

void UTabCreateGroupDialog::HandleConfirmClicked()
{
	if (IsInputValid())
	{
		OnGroupCreated.Broadcast(GetGroupName(), SelectedColor);
		CloseDialog();
	}
}

void UTabCreateGroupDialog::HandleCancelClicked()
{
	OnCancelled.Broadcast();
	CloseDialog();
}

// ============ Color Button Handlers ============

void UTabCreateGroupDialog::HandleColorRedClicked()
{
	SelectPresetColor(0);
}

void UTabCreateGroupDialog::HandleColorOrangeClicked()
{
	SelectPresetColor(1);
}

void UTabCreateGroupDialog::HandleColorYellowClicked()
{
	SelectPresetColor(2);
}

void UTabCreateGroupDialog::HandleColorGreenClicked()
{
	SelectPresetColor(3);
}

void UTabCreateGroupDialog::HandleColorCyanClicked()
{
	SelectPresetColor(4);
}

void UTabCreateGroupDialog::HandleColorBlueClicked()
{
	SelectPresetColor(5);
}

void UTabCreateGroupDialog::HandleColorPurpleClicked()
{
	SelectPresetColor(6);
}

void UTabCreateGroupDialog::HandleColorPinkClicked()
{
	SelectPresetColor(7);
}
