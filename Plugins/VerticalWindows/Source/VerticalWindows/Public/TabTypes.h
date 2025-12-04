#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TabTypes.generated.h"

/**
 * 编辑器标签页信息
 */
USTRUCT(BlueprintType)
struct VERTICALWINDOWS_API FEditorTabInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FString TabId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FString AssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FString AssetType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FString AssetClassName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	bool bIsDirty = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FString GroupId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FLinearColor GroupColor = FLinearColor::White;
};

/**
 * 标签分组数据
 */
USTRUCT(BlueprintType)
struct VERTICALWINDOWS_API FTabGroupInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FString GroupId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FString GroupName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	bool bExpanded = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	TArray<FEditorTabInfo> Tabs;
};