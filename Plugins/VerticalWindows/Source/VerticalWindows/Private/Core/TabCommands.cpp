#include "TabCommands.h"
#include "TabOpenCommand.h"
#include "TabCloseCommand.h"
#include "TabSaveCommand.h"
#include "ITabOperations.h"

// ============ Browse Command ============

UTabBrowseCommand* UTabBrowseCommand::Create(ITabOperations* Operations, const FEditorTabInfo& Tab)
{
	UTabBrowseCommand* Command = NewObject<UTabBrowseCommand>();
	Command->SetOperationsRef(Operations);
	Command->TargetTabs.Add(Tab);
	return Command;
}

bool UTabBrowseCommand::Execute()
{
	if (!OperationsRef || TargetTabs.Num() == 0) return false;

	OperationsRef->BrowseToAsset(TargetTabs[0].AssetPath);
	return true;
}

// ============ Add To Group Command ============

UTabAddToGroupCommand* UTabAddToGroupCommand::Create(ITabOperations* Operations, const TArray<FEditorTabInfo>& Tabs, const FString& GroupId)
{
	UTabAddToGroupCommand* Command = NewObject<UTabAddToGroupCommand>();
	Command->SetOperationsRef(Operations);
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
	if (!OperationsRef) return false;

	return OperationsRef->AssignTabsToGroup(TargetTabs, TargetGroupId);
}

bool UTabAddToGroupCommand::Undo()
{
	if (!OperationsRef) return false;

	// Restore previous groups
	for (const FEditorTabInfo& Tab : TargetTabs)
	{
		if (const FString* PrevGroup = PreviousGroupIds.Find(Tab.TabId))
		{
			TArray<FEditorTabInfo> SingleTab;
			SingleTab.Add(Tab);
			OperationsRef->AssignTabsToGroup(SingleTab, *PrevGroup);
		}
	}
	return true;
}

// ============ Move Command ============

UTabMoveCommand* UTabMoveCommand::Create(ITabOperations* Operations, const FEditorTabInfo& Tab, int32 NewIndex)
{
	UTabMoveCommand* Command = NewObject<UTabMoveCommand>();
	Command->SetOperationsRef(Operations);
	Command->TargetTabs.Add(Tab);
	Command->TargetIndex = NewIndex;
	Command->OriginalIndex = -1;
	return Command;
}

bool UTabMoveCommand::Execute()
{
	if (!OperationsRef || TargetTabs.Num() == 0) return false;

	OriginalIndex = OperationsRef->GetTabIndex(TargetTabs[0].TabId);
	return OperationsRef->MoveTabToIndex(TargetTabs[0].TabId, TargetIndex);
}

bool UTabMoveCommand::Undo()
{
	if (!OperationsRef || TargetTabs.Num() == 0 || OriginalIndex < 0) return false;

	return OperationsRef->MoveTabToIndex(TargetTabs[0].TabId, OriginalIndex);
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

// ============ Convenience Methods ============

bool UTabCommandInvoker::OpenTab(const FEditorTabInfo& Tab)
{
	if (!Operations) return false;
	UTabOpenCommand* Command = UTabOpenCommand::Create(Operations, Tab);
	return ExecuteCommand(Command);
}

bool UTabCommandInvoker::OpenTabs(const TArray<FEditorTabInfo>& Tabs)
{
	if (!Operations) return false;
	UTabOpenCommand* Command = UTabOpenCommand::Create(Operations, Tabs);
	return ExecuteCommand(Command);
}

bool UTabCommandInvoker::CloseTab(const FEditorTabInfo& Tab)
{
	if (!Operations) return false;
	UTabCloseCommand* Command = UTabCloseCommand::Create(Operations, Tab);
	return ExecuteCommand(Command);
}

bool UTabCommandInvoker::CloseTabs(const TArray<FEditorTabInfo>& Tabs)
{
	if (!Operations) return false;
	UTabCloseCommand* Command = UTabCloseCommand::Create(Operations, Tabs);
	return ExecuteCommand(Command);
}

bool UTabCommandInvoker::SaveTab(const FEditorTabInfo& Tab)
{
	if (!Operations) return false;
	UTabSaveCommand* Command = UTabSaveCommand::Create(Operations, Tab);
	return ExecuteCommand(Command);
}

bool UTabCommandInvoker::SaveTabs(const TArray<FEditorTabInfo>& Tabs)
{
	if (!Operations) return false;
	UTabSaveCommand* Command = UTabSaveCommand::Create(Operations, Tabs);
	return ExecuteCommand(Command);
}

bool UTabCommandInvoker::BrowseToAsset(const FEditorTabInfo& Tab)
{
	if (!Operations) return false;
	UTabBrowseCommand* Command = UTabBrowseCommand::Create(Operations, Tab);
	return ExecuteCommand(Command);
}

bool UTabCommandInvoker::AddToGroup(const TArray<FEditorTabInfo>& Tabs, const FString& GroupId)
{
	if (!Operations) return false;
	UTabAddToGroupCommand* Command = UTabAddToGroupCommand::Create(Operations, Tabs, GroupId);
	return ExecuteCommand(Command);
}
