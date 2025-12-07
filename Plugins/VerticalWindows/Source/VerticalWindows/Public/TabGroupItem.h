#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabTypes.h"
#include "TabGroupItem.generated.h"

class UImage;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGroupItemClicked, const FString&, GroupId);

/**
 * Group Item Widget - 群组子菜单中的单个群组项
 * 显示群组颜色图标和群组名称
 */
UCLASS(BlueprintType, Blueprintable)
class VERTICALWINDOWS_API UTabGroupItem : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============ Data ============

	/** 群组ID */
	UPROPERTY(BlueprintReadOnly, Category = "Group Item")
	FString GroupId;

	/** 群组名称 */
	UPROPERTY(BlueprintReadOnly, Category = "Group Item")
	FString GroupName;

	/** 群组颜色 */
	UPROPERTY(BlueprintReadOnly, Category = "Group Item")
	FLinearColor GroupColor;

	/** 是否为"新建群组"按钮 */
	UPROPERTY(BlueprintReadOnly, Category = "Group Item")
	bool bIsCreateNewButton = false;

	// ============ Component Bindings ============

	/** 颜色图标 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* ColorIcon;

	/** 群组名称文本 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* GroupNameText;

	/** 主按钮 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* RootButton;

	// ============ Events ============

	UPROPERTY(BlueprintAssignable, Category = "Group Item")
	FOnGroupItemClicked OnClicked;

	// ============ Methods ============

	/** 设置群组数据 */
	UFUNCTION(BlueprintCallable, Category = "Group Item")
	void SetGroupData(const FString& InGroupId, const FString& InGroupName, FLinearColor InColor);

	/** 设置为"新建群组"模式 */
	UFUNCTION(BlueprintCallable, Category = "Group Item")
	void SetAsCreateNewButton();

	/** UI更新事件 */
	UFUNCTION(BlueprintNativeEvent, Category = "Group Item")
	void OnDataUpdated();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleButtonClicked();
};
