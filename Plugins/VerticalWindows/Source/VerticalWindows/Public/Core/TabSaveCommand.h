#pragma once

#include "CoreMinimal.h"
#include "TabCommands.h"
#include "TabSaveCommand.generated.h"

/**
 * Save Tab Command - 保存标签
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabSaveCommand : public UTabCommandBase
{
	GENERATED_BODY()

public:
	/** Create for multiple tabs */
	static UTabSaveCommand* Create(ITabOperations* Operations, const TArray<FEditorTabInfo>& Tabs);

	/** Create for single tab */
	static UTabSaveCommand* Create(ITabOperations* Operations, const FEditorTabInfo& Tab);

	virtual bool Execute() override;
	virtual ETabCommandType GetCommandType() const override { return ETabCommandType::Save; }
};
