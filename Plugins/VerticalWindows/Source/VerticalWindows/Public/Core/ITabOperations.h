#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TabTypes.h"
#include "ITabOperations.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UTabOperations : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for tab operations
 * Commands depend on this interface, not on concrete UEUW_Windows
 * This breaks the circular dependency
 */
class VERTICALWINDOWS_API ITabOperations
{
	GENERATED_BODY()

public:
	// ============ Basic Operations ============

	/** Activate/Open a tab by ID */
	virtual bool ActivateTab(const FString& TabId) = 0;

	/** Close a tab by ID */
	virtual bool CloseTab(const FString& TabId) = 0;

	/** Save an asset */
	virtual bool SaveAsset(const FString& AssetPath) = 0;

	/** Browse to asset in content browser */
	virtual void BrowseToAsset(const FString& AssetPath) = 0;

	// ============ Group Operations ============

	/** Assign tabs to a group */
	virtual bool AssignTabsToGroup(const TArray<FEditorTabInfo>& Tabs, const FString& GroupId) = 0;

	// ============ Reorder Operations ============

	/** Get tab index by ID */
	virtual int32 GetTabIndex(const FString& TabId) const = 0;

	/** Move tab to new index */
	virtual bool MoveTabToIndex(const FString& TabId, int32 NewIndex) = 0;
};
