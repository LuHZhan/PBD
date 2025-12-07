#include "TabCommands.h"
#include "EUW_Windows.h"

// ============ Open Command ============

UTabOpenCommand* UTabOpenCommand::Create(UEUW_Windows* Windows, const TArray<FEditorTabInfo>& Tabs)
{
	UTabOpenCommand* Command = NewObject<UTabOpenCommand>();
	Command->SetWindowsRef(Windows);
	Command->TargetTabs = Tabs;
	return Command;
}

bool UTabOpenCommand::Execute()
{
	if (!WindowsRef.IsValid()) return false;

	bool bSuccess = true;
	for (const FEditorTabInfo& Tab : TargetTabs)
	{
		bSuccess &= WindowsRef->ActivateTab(Tab.TabId);
	}
	return bSuccess;
}

// ============ Close Command ============

UTabCloseCommand* UTabCloseCommand::Create(UEUW_Windows* Windows, const TArray<FEditorTabInfo>& Tabs)
{
	UTabCloseCommand* Command = NewObject<UTabCloseCommand>();
	Command->SetWindowsRef(Windows);
	Command->TargetTabs = Tabs;
	return Command;
}

bool UTabCloseCommand::Execute()
{
	if (!WindowsRef.IsValid()) return false;

	bool bSuccess = true;
	for (const FEditorTabInfo& Tab : TargetTabs)
	{
		bSuccess &= WindowsRef->CloseTab(Tab.TabId);
	}
	return bSuccess;
}

// ============ Save Command ============

UTabSaveCommand* UTabSaveCommand::Create(UEUW_Windows* Windows, const TArray<FEditorTabInfo>& Tabs)
{
	UTabSaveCommand* Command = NewObject<UTabSaveCommand>();
	Command->SetWindowsRef(Windows);
	Command->TargetTabs = Tabs;
	return Command;
}

bool UTabSaveCommand::Execute()
{
	if (!WindowsRef.IsValid()) return false;

	bool bSuccess = true;
	for (const FEditorTabInfo& Tab : TargetTabs)
	{
		if (Tab.bIsDirty)
		{
			bSuccess &= WindowsRef->SaveAsset(Tab.AssetPath);
		}
	}
	return bSuccess;
}

// ============ Browse Command ============

UTabBrowseCommand* UTabBrowseCommand::Create(UEUW_Windows* Windows, const FEditorTabInfo& Tab)
{
	UTabBrowseCommand* Command = NewObject<UTabBrowseCommand>();
	Command->SetWindowsRef(Windows);
	Command->TargetTabs.Add(Tab);
	return Command;
}

bool UTabBrowseCommand::Execute()
{
	if (!WindowsRef.IsValid() || TargetTabs.Num() == 0) return false;

	WindowsRef->BrowseToAsset(TargetTabs[0].AssetPath);
	return true;
}

// ============ Add To Group Command ============

UTabAddToGroupCommand* UTabAddToGroupCommand::Create(UEUW_Windows* Windows, const TArray<FEditorTabInfo>& Tabs, const FString& GroupId)
{
	UTabAddToGroupCommand* Command = NewObject<UTabAddToGroupCommand>();
	Command->SetWindowsRef(Windows);
	Command->TargetTabs = Tabs;
	Command->TargetGroupId = GroupId;

	// Store previous group IDs for undo
	for (const FEditorTabInfo& Tab : Tabs)
	{
		Command->PreviousGroupIds.Add(Tab.TabId, Tab.GroupId);
	}

	return Command;
}

bool UTabAddToGroupCommand::Execute()
{
	if (!WindowsRef.IsValid()) return false;

	// Update group assignment via Windows
	return WindowsRef->AssignTabsToGroup(TargetTabs, TargetGroupId);
}

bool UTabAddToGroupCommand::Undo()
{
	if (!WindowsRef.IsValid()) return false;

	// Restore previous groups
	for (const FEditorTabInfo& Tab : TargetTabs)
	{
		if (const FString* PrevGroup = PreviousGroupIds.Find(Tab.TabId))
		{
			TArray<FEditorTabInfo> SingleTab;
			SingleTab.Add(Tab);
			WindowsRef->AssignTabsToGroup(SingleTab, *PrevGroup);
		}
	}
	return true;
}

// ============ Move Command ============

UTabMoveCommand* UTabMoveCommand::Create(UEUW_Windows* Windows, const FEditorTabInfo& Tab, int32 NewIndex)
{
	UTabMoveCommand* Command = NewObject<UTabMoveCommand>();
	Command->SetWindowsRef(Windows);
	Command->TargetTabs.Add(Tab);
	Command->TargetIndex = NewIndex;
	Command->OriginalIndex = -1; // Will be set during execute
	return Command;
}

bool UTabMoveCommand::Execute()
{
	if (!WindowsRef.IsValid() || TargetTabs.Num() == 0) return false;

	OriginalIndex = WindowsRef->GetTabIndex(TargetTabs[0].TabId);
	return WindowsRef->MoveTabToIndex(TargetTabs[0].TabId, TargetIndex);
}

bool UTabMoveCommand::Undo()
{
	if (!WindowsRef.IsValid() || TargetTabs.Num() == 0 || OriginalIndex < 0) return false;

	return WindowsRef->MoveTabToIndex(TargetTabs[0].TabId, OriginalIndex);
}

// ============ Command Invoker ============

bool UTabCommandInvoker::ExecuteCommand(UTabCommandBase* Command)
{
	if (!Command) return false;

	bool bSuccess = Command->Execute();

	if (bSuccess && Command->CanUndo())
	{
		CommandHistory.Add(Command);

		// Limit history size
		while (CommandHistory.Num() > MaxHistorySize)
		{
			CommandHistory.RemoveAt(0);
		}
	}

	return bSuccess;
}

bool UTabCommandInvoker::UndoLastCommand()
{
	if (CommandHistory.Num() == 0) return false;

	UTabCommandBase* LastCommand = CommandHistory.Pop();
	return LastCommand->Undo();
}

bool UTabCommandInvoker::CanUndo() const
{
	return CommandHistory.Num() > 0;
}

void UTabCommandInvoker::ClearHistory()
{
	CommandHistory.Empty();
}
