#pragma once

#include "CoreMinimal.h"
#include "TabTypes.h"
#include "TabCommands.generated.h"

class ITabOperations;

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
 * 
 * Note: Commands depend on ITabOperations interface, not concrete UEUW_Windows
 * This breaks circular dependency between Commands and Windows
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

	/** Set operations interface reference */
	void SetOperationsRef(ITabOperations* InOperations) { OperationsRef = InOperations; }

protected:
	UPROPERTY()
	TArray<FEditorTabInfo> TargetTabs;

	/** Interface reference - not UPROPERTY because it's an interface pointer */
	ITabOperations* OperationsRef = nullptr;
};

/**
 * Browse To Asset Command - 在内容浏览器中定位资产
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabBrowseCommand : public UTabCommandBase
{
	GENERATED_BODY()

public:
	static UTabBrowseCommand* Create(ITabOperations* Operations, const FEditorTabInfo& Tab);

	virtual bool Execute() override;
	virtual ETabCommandType GetCommandType() const override { return ETabCommandType::BrowseToAsset; }
};

/**
 * Add To Group Command - 添加到群组（可撤销）
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabAddToGroupCommand : public UTabCommandBase
{
	GENERATED_BODY()

public:
	static UTabAddToGroupCommand* Create(ITabOperations* Operations, const TArray<FEditorTabInfo>& Tabs, const FString& GroupId);

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
 * Move Tab Command - 移动顺序（可撤销）
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabMoveCommand : public UTabCommandBase
{
	GENERATED_BODY()

public:
	static UTabMoveCommand* Create(ITabOperations* Operations, const FEditorTabInfo& Tab, int32 NewIndex);

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
	/** Set operations interface */
	void SetOperations(ITabOperations* InOperations) { Operations = InOperations; }

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

	// ============ Convenience Methods ============

	/** Execute Open command for single tab */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	bool OpenTab(const FEditorTabInfo& Tab);

	/** Execute Open command for multiple tabs */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	bool OpenTabs(const TArray<FEditorTabInfo>& Tabs);

	/** Execute Close command for single tab */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	bool CloseTab(const FEditorTabInfo& Tab);

	/** Execute Close command for multiple tabs */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	bool CloseTabs(const TArray<FEditorTabInfo>& Tabs);

	/** Execute Save command for single tab */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	bool SaveTab(const FEditorTabInfo& Tab);

	/** Execute Save command for multiple tabs */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	bool SaveTabs(const TArray<FEditorTabInfo>& Tabs);

	/** Execute Browse command */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	bool BrowseToAsset(const FEditorTabInfo& Tab);

	/** Execute AddToGroup command */
	UFUNCTION(BlueprintCallable, Category = "Tab Command")
	bool AddToGroup(const TArray<FEditorTabInfo>& Tabs, const FString& GroupId);

private:
	UPROPERTY()
	TArray<UTabCommandBase*> CommandHistory;

	ITabOperations* Operations = nullptr;

	static const int32 MaxHistorySize = 50;
};
