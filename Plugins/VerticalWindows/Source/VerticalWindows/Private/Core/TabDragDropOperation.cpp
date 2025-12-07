#include "TabDragDropOperation.h"
#include "TabItemWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

UTabDragDropOperation* UTabDragDropOperation::CreateTabDragOperation(
	UObject* WorldContextObject,
	UTabItemWidget* SourceItemWidget,
	const FEditorTabInfo& TabInfo,
	int32 InOriginalIndex)
{
	if (!SourceItemWidget) return nullptr;

	UTabDragDropOperation* Operation = NewObject<UTabDragDropOperation>();
	Operation->DraggedTab = TabInfo;
	Operation->SourceWidget = SourceItemWidget;
	Operation->OriginalIndex = InOriginalIndex;
	Operation->bIsValidDrag = true;

	// Set the visual (the dragged widget appearance)
	Operation->DefaultDragVisual = SourceItemWidget;
	Operation->Pivot = EDragPivot::CenterCenter;

	return Operation;
}
