#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "TabTypes.h"
#include "EUW_Windows.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEditorTabsChanged);

UCLASS(BlueprintType)
class VERTICALWINDOWS_API UEUW_Windows : public UEditorUtilityWidget
{
    GENERATED_BODY()

public:
    UEUW_Windows();

    // ============ 获取标签页 ============

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    TArray<FEditorTabInfo> GetAllOpenTabs();

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    TArray<FTabGroupInfo> GetGroupedTabs();

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    TArray<FEditorTabInfo> GetTabsByType(const FString& AssetType);

    // ============ 操作标签页 ============

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    bool ActivateTab(const FString& TabId);

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    bool CloseTab(const FString& TabId);

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    void BrowseToAsset(const FString& AssetPath);

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    bool SaveAsset(const FString& AssetPath);

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    void CloseAllTabs();

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    void SaveAllDirtyAssets();

    // ============ 刷新 ============

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    void RefreshTabs();

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    void StartAutoRefresh(float IntervalSeconds = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    void StopAutoRefresh();

    // ============ 工具 ============

    UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
    TMap<FString, FLinearColor> GetGroupColorMap();

    UFUNCTION(BlueprintPure, Category = "Vertical Windows")
    static FLinearColor ParseHexColor(const FString& HexColor);

    // ============ 事件 ============

    UPROPERTY(BlueprintAssignable, Category = "Vertical Windows")
    FOnEditorTabsChanged OnTabsChanged;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    void InternalRefresh();
    FString GetAssetTypeDisplayName(UClass* AssetClass);
    FLinearColor GetAssetTypeColor(const FString& AssetType);
    void InitGroupColors();

    FTimerHandle AutoRefreshTimerHandle;
    TArray<FEditorTabInfo> CachedTabs;
    TMap<FString, FLinearColor> GroupColors;
};