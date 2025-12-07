#include "TabSaveCommand.h"
#include "ITabOperations.h"

UTabSaveCommand* UTabSaveCommand::Create(ITabOperations* Operations, const TArray<FEditorTabInfo>& Tabs)
{
	UTabSaveCommand* Command = NewObject<UTabSaveCommand>();
	Command->SetOperationsRef(Operations);
	Command->TargetTabs = Tabs;
	return Command;
}

UTabSaveCommand* UTabSaveCommand::Create(ITabOperations* Operations, const FEditorTabInfo& Tab)
{
	TArray<FEditorTabInfo> Tabs;
	Tabs.Add(Tab);
	return Create(Operations, Tabs);
}

bool UTabSaveCommand::Execute()
{
	if (!OperationsRef) return false;

	bool bSuccess = true;
	for (const FEditorTabInfo& Tab : TargetTabs)
	{
		if (Tab.bIsDirty)
		{
			bSuccess &= OperationsRef->SaveAsset(Tab.AssetPath);
		}
	}
	return bSuccess;
}
