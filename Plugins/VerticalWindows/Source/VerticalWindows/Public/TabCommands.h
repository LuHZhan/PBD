#pragma once

#include "CoreMinimal.h"
#include "TabTypes.h"
#include "TabCommands.generated.h"

class UEUW_Windows;

/**
 * Command Type Enum
 */
UENUM(BlueprintType)
enum class ETabCommandType : uint8
{
	None,
	Open,
	Close,
	Save,
	BrowseToAsset,
	AddToGroup,
	RemoveFromGroup,
	MoveToIndex
};

/**
 * Base Command Interface - Command Pattern
 * Encapsulates operations as objects for easy extension and batch execution
 */
UCLASS(Abstract, BlueprintType)
class VERTICALWINDOWS_API UTabCommandBase : public UObject
{
	GENERATED_BODY()

public:
	/** Execute the command */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	virtual bool Execute() PURE_VIRTUAL(UTabCommandBase::Execute, return false;);

	/** Undo the command (optional) */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	virtual bool Undo() { return false; }

	/** Can this command be undone */
	UFUNCTION(BlueprintPure, Category = "Tab Command")
	virtual bool CanUndo() const { return false; }

	/** Get command type */
	UFUNCTION(BlueprintPure, Category = "Tab Command")
	virtual ETabCommandType GetCommandType() const { return ETabCommandType::None; }

	/** Get target tabs */
	UFUNCTION(BlueprintPure, Category = "Tab Command")
	TArray<FEditorTabInfo> GetTargetTabs() const { return TargetTabs; }

	/** Set windows reference */
	void SetWindowsRef(UEUW_Windows* InWindows) { WindowsRef = InWindows; }

protected:
	UPROPERTY()
	TArray<FEditorTabInfo> TargetTabs;

	UPROPERTY()
	TWeakObjectPtr<UEUW_Windows> WindowsRef;
};

/**
 * Open Tab Command
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabOpenCommand : public UTabCommandBase
{
	GENERATED_BODY()

public:
	static UTabOpenCommand* Create(UEUW_Windows* Windows, const TArray<FEditorTabInfo>& Tabs);

	virtual bool Execute() override;
	virtual ETabCommandType GetCommandType() const override { return ETabCommandType::Open; }
};

/**
 * Close Tab Command
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabCloseCommand : public UTabCommandBase
{
	GENERATED_BODY()

public:
	static UTabCloseCommand* Create(UEUW_Windows* Windows, const TArray<FEditorTabInfo>& Tabs);

	virtual bool Execute() override;
	virtual ETabCommandType GetCommandType() const override { return ETabCommandType::Close; }
};

/**
 * Save Tab Command
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabSaveCommand : public UTabCommandBase
{
	GENERATED_BODY()

public:
	static UTabSaveCommand* Create(UEUW_Windows* Windows, const TArray<FEditorTabInfo>& Tabs);

	virtual bool Execute() override;
	virtual ETabCommandType GetCommandType() const override { return ETabCommandType::Save; }
};

/**
 * Browse To Asset Command
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabBrowseCommand : public UTabCommandBase
{
	GENERATED_BODY()

public:
	static UTabBrowseCommand* Create(UEUW_Windows* Windows, const FEditorTabInfo& Tab);

	virtual bool Execute() override;
	virtual ETabCommandType GetCommandType() const override { return ETabCommandType::BrowseToAsset; }
};

/**
 * Add To Group Command
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabAddToGroupCommand : public UTabCommandBase
{
	GENERATED_BODY()

public:
	static UTabAddToGroupCommand* Create(UEUW_Windows* Windows, const TArray<FEditorTabInfo>& Tabs, const FString& GroupId);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual bool CanUndo() const override { return true; }
	virtual ETabCommandType GetCommandType() const override { return ETabCommandType::AddToGroup; }

	UPROPERTY()
	FString TargetGroupId;

	UPROPERTY()
	TMap<FString, FString> PreviousGroupIds;
};

/**
 * Move Tab Command (for drag reordering)
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabMoveCommand : public UTabCommandBase
{
	GENERATED_BODY()

public:
	static UTabMoveCommand* Create(UEUW_Windows* Windows, const FEditorTabInfo& Tab, int32 NewIndex);

	virtual bool Execute() override;
	virtual bool Undo() override;
	virtual bool CanUndo() const override { return true; }
	virtual ETabCommandType GetCommandType() const override { return ETabCommandType::MoveToIndex; }

	UPROPERTY()
	int32 TargetIndex;

	UPROPERTY()
	int32 OriginalIndex;
};

/**
 * Command Invoker - Executes and manages command history
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabCommandInvoker : public UObject
{
	GENERATED_BODY()

public:
	/** Execute a command */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	bool ExecuteCommand(UTabCommandBase* Command);

	/** Undo last command */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	bool UndoLastCommand();

	/** Can undo */
	UFUNCTION(BlueprintPure, Category = "Tab Command")
	bool CanUndo() const;

	/** Clear history */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	void ClearHistory();

private:
	UPROPERTY()
	TArray<UTabCommandBase*> CommandHistory;

	static const int32 MaxHistorySize = 50;
};
