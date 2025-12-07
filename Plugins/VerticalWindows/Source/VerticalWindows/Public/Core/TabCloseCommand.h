#pragma once

#include "CoreMinimal.h"
#include "TabCommands.h"
#include "TabCloseCommand.generated.h"

/**
 * Close Tab Command - 关闭标签
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabCloseCommand : public UTabCommandBase
{
	GENERATED_BODY()

public:
	/** Create for multiple tabs */
	static UTabCloseCommand* Create(ITabOperations* Operations, const TArray<FEditorTabInfo>& Tabs);

	/** Create for single tab */
	static UTabCloseCommand* Create(ITabOperations* Operations, const FEditorTabInfo& Tab);

	virtual bool Execute() override;
	virtual ETabCommandType GetCommandType() const override { return ETabCommandType::Close; }
};
