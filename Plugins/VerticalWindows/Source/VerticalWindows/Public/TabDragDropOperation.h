#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "TabTypes.h"
#include "TabDragDropOperation.generated.h"

class UTabItemWidget;

/**
 * Drag Drop Operation for Tab Items
 * Handles visual feedback and data transfer during drag
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** The tab data being dragged */
	UPROPERTY(BlueprintReadOnly, Category = "Tab Drag")
	FEditorTabInfo DraggedTab;

	/** Source widget */
	UPROPERTY(BlueprintReadOnly, Category = "Tab Drag")
	TWeakObjectPtr<UTabItemWidget> SourceWidget;

	/** Original index before drag */
	UPROPERTY(BlueprintReadOnly, Category = "Tab Drag")
	int32 OriginalIndex;

	/** Is this a valid drag operation */
	UPROPERTY(BlueprintReadOnly, Category = "Tab Drag")
	bool bIsValidDrag;

	/** Create drag operation */
	UFUNCTION(BlueprintCallable, Category = "Tab Drag", meta = (WorldContext = "WorldContextObject"))
	static UTabDragDropOperation* CreateTabDragOperation(
		UObject* WorldContextObject,
		UTabItemWidget* SourceItemWidget,
		const FEditorTabInfo& TabInfo,
		int32 InOriginalIndex);
};

/**
 * Interface for widgets that can receive tab drops
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UTabDropTarget : public UInterface
{
	GENERATED_BODY()
};

class ITabDropTarget
{
	GENERATED_BODY()

public:
	/** Called when drag enters this widget */
	virtual void OnTabDragEnter(UTabDragDropOperation* Operation) = 0;

	/** Called when drag leaves this widget */
	virtual void OnTabDragLeave(UTabDragDropOperation* Operation) = 0;

	/** Called when tab is dropped on this widget */
	virtual bool OnTabDrop(UTabDragDropOperation* Operation, int32 DropIndex) = 0;

	/** Get the drop index based on cursor position */
	virtual int32 GetDropIndex(const FGeometry& Geometry, const FVector2D& ScreenPosition) = 0;
};
