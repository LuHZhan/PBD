#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Styling/SlateBrush.h"
#include "TabTypes.generated.h"

// ============ NEW: Item State Enum ============
/**
 * Tab Item Visual State
 */
UENUM(BlueprintType)
enum class ETabItemState : uint8
{
	Normal,
	Hovered,
	Selected,
	Dragging
};

// ============ NEW: Click Type Enum ============
/**
 * Mouse Click Type
 */
UENUM(BlueprintType)
enum class ETabClickType : uint8
{
	LeftClick,
	RightClick,
	MiddleClick
};

/**
 * Editor Tab Info
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

	/** Asset type icon brush */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FSlateBrush IconBrush;

	// ============ NEW: Additional fields for ordering ============
	
	/** Display order index (for custom sorting) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	int32 DisplayOrder = 0;

	/** Custom group ID (user-defined groups) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	FString CustomGroupId;

	// ============ Equality operator for comparison ============
	bool operator==(const FEditorTabInfo& Other) const
	{
		return TabId == Other.TabId;
	}
};

/**
 * Tab Group Data
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

	// ============ NEW: Custom group flag ============

	/** Is this a user-created custom group */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab")
	bool bIsCustomGroup = false;
};

// ============ NEW: Custom Group Data for persistence ============
/**
 * Custom Group Definition (user-created groups like browser tab groups)
 */
USTRUCT(BlueprintType)
struct VERTICALWINDOWS_API FCustomTabGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Group")
	FString GroupId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Group")
	FString GroupName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Group")
	FLinearColor Color = FLinearColor::Gray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Group")
	TArray<FString> TabIds;
};
