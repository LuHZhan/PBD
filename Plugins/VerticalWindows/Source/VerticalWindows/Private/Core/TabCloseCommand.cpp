#include "TabCloseCommand.h"
#include "ITabOperations.h"

UTabCloseCommand* UTabCloseCommand::Create(ITabOperations* Operations, const TArray<FEditorTabInfo>& Tabs)
{
	UTabCloseCommand* Command = NewObject<UTabCloseCommand>();
	Command->SetOperationsRef(Operations);
	Command->TargetTabs = Tabs;
	return Command;
}

UTabCloseCommand* UTabCloseCommand::Create(ITabOperations* Operations, const FEditorTabInfo& Tab)
{
	TArray<FEditorTabInfo> Tabs;
	Tabs.Add(Tab);
	return Create(Operations, Tabs);
}

bool UTabCloseCommand::Execute()
{
	if (!OperationsRef) return false;

	bool bSuccess = true;
	for (const FEditorTabInfo& Tab : TargetTabs)
	{
		bSuccess &= OperationsRef->CloseTab(Tab.TabId);
	}
	return bSuccess;
}
