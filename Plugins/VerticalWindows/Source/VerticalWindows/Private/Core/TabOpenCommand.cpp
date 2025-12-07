#include "TabOpenCommand.h"
#include "ITabOperations.h"

UTabOpenCommand* UTabOpenCommand::Create(ITabOperations* Operations, const TArray<FEditorTabInfo>& Tabs)
{
	UTabOpenCommand* Command = NewObject<UTabOpenCommand>();
	Command->SetOperationsRef(Operations);
	Command->TargetTabs = Tabs;
	return Command;
}

UTabOpenCommand* UTabOpenCommand::Create(ITabOperations* Operations, const FEditorTabInfo& Tab)
{
	TArray<FEditorTabInfo> Tabs;
	Tabs.Add(Tab);
	return Create(Operations, Tabs);
}

bool UTabOpenCommand::Execute()
{
	if (!OperationsRef) return false;

	bool bSuccess = true;
	for (const FEditorTabInfo& Tab : TargetTabs)
	{
		bSuccess &= OperationsRef->ActivateTab(Tab.TabId);
	}
	return bSuccess;
}
