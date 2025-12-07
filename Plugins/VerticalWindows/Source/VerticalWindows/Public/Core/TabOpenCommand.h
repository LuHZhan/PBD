#pragma once

#include "CoreMinimal.h"
#include "TabCommands.h"
#include "TabOpenCommand.generated.h"

/**
 * Open Tab Command - 打开/激活标签
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabOpenCommand : public UTabCommandBase
{
	GENERATED_BODY()

public:
	/** Create for multiple tabs */
	static UTabOpenCommand* Create(ITabOperations* Operations, const TArray<FEditorTabInfo>& Tabs);

	/** Create for single tab */
	static UTabOpenCommand* Create(ITabOperations* Operations, const FEditorTabInfo& Tab);

	virtual bool Execute() override;
	virtual ETabCommandType GetCommandType() const override { return ETabCommandType::Open; }
};
