#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabCreateGroupDialog.generated.h"

class UEditableTextBox;
class UButton;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGroupCreated, const FString&, GroupName, FLinearColor, GroupColor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogCancelled);

/**
 * Create Group Dialog - 创建新群组的对话框
 * 包含名称输入框和颜色选择
 */
UCLASS(BlueprintType, Blueprintable)
class VERTICALWINDOWS_API UTabCreateGroupDialog : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============ Component Bindings ============

	/** 群组名称输入框 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UEditableTextBox* NameInputBox;

	/** 当前选中的颜色预览 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* ColorPreview;

	/** 确认按钮 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* ConfirmButton;

	/** 取消按钮 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* CancelButton;

	// ============ Color Palette Buttons (Optional) ============

	/** 颜色选择按钮 - 红色 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UButton* ColorBtn_Red;

	/** 颜色选择按钮 - 橙色 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UButton* ColorBtn_Orange;

	/** 颜色选择按钮 - 黄色 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UButton* ColorBtn_Yellow;

	/** 颜色选择按钮 - 绿色 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UButton* ColorBtn_Green;

	/** 颜色选择按钮 - 青色 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UButton* ColorBtn_Cyan;

	/** 颜色选择按钮 - 蓝色 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UButton* ColorBtn_Blue;

	/** 颜色选择按钮 - 紫色 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UButton* ColorBtn_Purple;

	/** 颜色选择按钮 - 粉色 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UButton* ColorBtn_Pink;

	// ============ Data ============

	/** 当前选中的颜色 */
	UPROPERTY(BlueprintReadWrite, Category = "Create Group Dialog")
	FLinearColor SelectedColor;

	/** 预设颜色列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Create Group Dialog")
	TArray<FLinearColor> PresetColors;

	// ============ Events ============

	/** 确认创建群组 */
	UPROPERTY(BlueprintAssignable, Category = "Create Group Dialog")
	FOnGroupCreated OnGroupCreated;

	/** 取消对话框 */
	UPROPERTY(BlueprintAssignable, Category = "Create Group Dialog")
	FOnDialogCancelled OnCancelled;

	// ============ Methods ============

	/** 显示对话框 */
	UFUNCTION(BlueprintCallable, Category = "Create Group Dialog")
	void ShowDialog();

	/** 关闭对话框 */
	UFUNCTION(BlueprintCallable, Category = "Create Group Dialog")
	void CloseDialog();

	/** 选择颜色 */
	UFUNCTION(BlueprintCallable, Category = "Create Group Dialog")
	void SelectColor(FLinearColor NewColor);

	/** 选择预设颜色（通过索引） */
	UFUNCTION(BlueprintCallable, Category = "Create Group Dialog")
	void SelectPresetColor(int32 ColorIndex);

	/** 获取输入的群组名称 */
	UFUNCTION(BlueprintPure, Category = "Create Group Dialog")
	FString GetGroupName() const;

	/** 验证输入是否有效 */
	UFUNCTION(BlueprintPure, Category = "Create Group Dialog")
	bool IsInputValid() const;

	/** 初始化预设颜色 */
	UFUNCTION(BlueprintCallable, Category = "Create Group Dialog")
	void InitializePresetColors();

	// ============ Blueprint Events ============

	/** 对话框打开时 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Create Group Dialog")
	void OnDialogOpened();

	/** 对话框关闭时 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Create Group Dialog")
	void OnDialogClosed();

	/** 颜色改变时 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Create Group Dialog")
	void OnColorChanged(FLinearColor NewColor);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleConfirmClicked();

	UFUNCTION()
	void HandleCancelClicked();

	// ============ Color Button Handlers ============

	UFUNCTION()
	void HandleColorRedClicked();

	UFUNCTION()
	void HandleColorOrangeClicked();

	UFUNCTION()
	void HandleColorYellowClicked();

	UFUNCTION()
	void HandleColorGreenClicked();

	UFUNCTION()
	void HandleColorCyanClicked();

	UFUNCTION()
	void HandleColorBlueClicked();

	UFUNCTION()
	void HandleColorPurpleClicked();

	UFUNCTION()
	void HandleColorPinkClicked();

private:
	void BindColorButtons();
	void UpdateColorPreview();
};
