#include "EUW_Windows.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "FileHelpers.h"

UEUW_Windows::UEUW_Windows()
{
    InitGroupColors();
}

void UEUW_Windows::NativeConstruct()
{
    Super::NativeConstruct();
    RefreshTabs();
    StartAutoRefresh(1.0f);
}

void UEUW_Windows::NativeDestruct()
{
    StopAutoRefresh();
    Super::NativeDestruct();
}

void UEUW_Windows::InitGroupColors()
{
    GroupColors.Empty();
    GroupColors.Add(TEXT("蓝图"), FLinearColor(0.29f, 0.56f, 0.85f, 1.0f));
    GroupColors.Add(TEXT("控件蓝图"), FLinearColor(0.61f, 0.35f, 0.71f, 1.0f));
    GroupColors.Add(TEXT("动画蓝图"), FLinearColor(0.75f, 0.22f, 0.17f, 1.0f));
    GroupColors.Add(TEXT("材质"), FLinearColor(0.15f, 0.68f, 0.38f, 1.0f));
    GroupColors.Add(TEXT("材质实例"), FLinearColor(0.18f, 0.80f, 0.44f, 1.0f));
    GroupColors.Add(TEXT("纹理"), FLinearColor(0.90f, 0.49f, 0.13f, 1.0f));
    GroupColors.Add(TEXT("静态网格体"), FLinearColor(0.20f, 0.60f, 0.86f, 1.0f));
    GroupColors.Add(TEXT("骨骼网格体"), FLinearColor(0.10f, 0.74f, 0.61f, 1.0f));
    GroupColors.Add(TEXT("动画序列"), FLinearColor(0.91f, 0.30f, 0.24f, 1.0f));
    GroupColors.Add(TEXT("动画蒙太奇"), FLinearColor(0.91f, 0.30f, 0.24f, 1.0f));
    GroupColors.Add(TEXT("音频"), FLinearColor(0.95f, 0.61f, 0.07f, 1.0f));
    GroupColors.Add(TEXT("Niagara系统"), FLinearColor(0.56f, 0.27f, 0.68f, 1.0f));
    GroupColors.Add(TEXT("关卡"), FLinearColor(0.17f, 0.24f, 0.31f, 1.0f));
    GroupColors.Add(TEXT("数据表"), FLinearColor(0.09f, 0.63f, 0.52f, 1.0f));
    GroupColors.Add(TEXT("曲线"), FLinearColor(0.95f, 0.77f, 0.06f, 1.0f));
    GroupColors.Add(TEXT("其他"), FLinearColor(0.50f, 0.55f, 0.55f, 1.0f));
}

TArray<FEditorTabInfo> UEUW_Windows::GetAllOpenTabs()
{
    TArray<FEditorTabInfo> Tabs;
    
    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    if (!AssetEditorSubsystem) return Tabs;
    
    TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
    
    for (UObject* Asset : EditedAssets)
    {
        if (!Asset) continue;
        
        FEditorTabInfo TabInfo;
        TabInfo.TabId = Asset->GetPathName();
        TabInfo.DisplayName = Asset->GetName();
        TabInfo.AssetPath = Asset->GetPathName();
        TabInfo.AssetClassName = Asset->GetClass()->GetName();
        TabInfo.AssetType = GetAssetTypeDisplayName(Asset->GetClass());
        TabInfo.bIsDirty = Asset->GetPackage()->IsDirty();
        TabInfo.bIsActive = false;
        TabInfo.GroupId = TabInfo.AssetType;
        TabInfo.GroupColor = GetAssetTypeColor(TabInfo.AssetType);
        
        Tabs.Add(TabInfo);
    }
    
    // 排序
    Tabs.Sort([](const FEditorTabInfo& A, const FEditorTabInfo& B)
    {
        if (A.GroupId != B.GroupId) return A.GroupId < B.GroupId;
        return A.DisplayName < B.DisplayName;
    });
    
    CachedTabs = Tabs;
    return Tabs;
}

TArray<FTabGroupInfo> UEUW_Windows::GetGroupedTabs()
{
    TArray<FEditorTabInfo> AllTabs = GetAllOpenTabs();
    TMap<FString, FTabGroupInfo> GroupMap;
    
    for (const FEditorTabInfo& Tab : AllTabs)
    {
        FString GroupId = Tab.GroupId.IsEmpty() ? TEXT("其他") : Tab.GroupId;
        
        if (!GroupMap.Contains(GroupId))
        {
            FTabGroupInfo NewGroup;
            NewGroup.GroupId = GroupId;
            NewGroup.GroupName = GroupId;
            NewGroup.Color = GetAssetTypeColor(GroupId);
            NewGroup.bExpanded = true;
            GroupMap.Add(GroupId, NewGroup);
        }
        GroupMap[GroupId].Tabs.Add(Tab);
    }
    
    TArray<FTabGroupInfo> Groups;
    GroupMap.GenerateValueArray(Groups);
    
    // 按预定义顺序排序
    static TMap<FString, int32> OrderMap = {
        {TEXT("蓝图"), 0}, {TEXT("控件蓝图"), 1}, {TEXT("动画蓝图"), 2},
        {TEXT("材质"), 3}, {TEXT("材质实例"), 4}, {TEXT("纹理"), 5},
        {TEXT("静态网格体"), 6}, {TEXT("骨骼网格体"), 7},
        {TEXT("动画序列"), 8}, {TEXT("动画蒙太奇"), 9},
        {TEXT("音频"), 10}, {TEXT("Niagara系统"), 11},
        {TEXT("关卡"), 12}, {TEXT("数据表"), 13}, {TEXT("曲线"), 14},
        {TEXT("其他"), 99}
    };
    
    Groups.Sort([](const FTabGroupInfo& A, const FTabGroupInfo& B)
    {
        int32 OrderA = OrderMap.Contains(A.GroupId) ? OrderMap[A.GroupId] : 50;
        int32 OrderB = OrderMap.Contains(B.GroupId) ? OrderMap[B.GroupId] : 50;
        return OrderA < OrderB;
    });
    
    return Groups;
}

TArray<FEditorTabInfo> UEUW_Windows::GetTabsByType(const FString& AssetType)
{
    TArray<FEditorTabInfo> FilteredTabs;
    for (const FEditorTabInfo& Tab : CachedTabs)
    {
        if (Tab.AssetType == AssetType) FilteredTabs.Add(Tab);
    }
    return FilteredTabs;
}

bool UEUW_Windows::ActivateTab(const FString& TabId)
{
    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    if (!AssetEditorSubsystem) return false;
    
    TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
    for (UObject* Asset : EditedAssets)
    {
        if (Asset && Asset->GetPathName() == TabId)
        {
            AssetEditorSubsystem->OpenEditorForAsset(Asset);
            return true;
        }
    }
    
    // 尝试加载并打开
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(TabId));
    
    if (AssetData.IsValid())
    {
        GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(AssetData.GetAsset());
        return true;
    }
    
    return false;
}

bool UEUW_Windows::CloseTab(const FString& TabId)
{
    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    if (!AssetEditorSubsystem) return false;
    
    TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
    
    for (UObject* Asset : EditedAssets)
    {
        if (Asset && Asset->GetPathName() == TabId)
        {
            AssetEditorSubsystem->CloseAllEditorsForAsset(Asset);
            // 延迟刷新...
            return true;
        }
    }
    return false;
}

void UEUW_Windows::BrowseToAsset(const FString& AssetPath)
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(AssetPath));
    
    if (AssetData.IsValid())
    {
        TArray<FAssetData> Assets;
        Assets.Add(AssetData);
        FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
        ContentBrowserModule.Get().SyncBrowserToAssets(Assets);
    }
}

bool UEUW_Windows::SaveAsset(const FString& AssetPath)
{
    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    if (!AssetEditorSubsystem) return false;
    
    TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
    
    for (UObject* Asset : EditedAssets)
    {
        if (Asset && Asset->GetPathName() == AssetPath && Asset->GetPackage()->IsDirty())
        {
            TArray<UPackage*> PackagesToSave;
            PackagesToSave.Add(Asset->GetPackage());
            FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
            return true;
        }
    }
    return false;
}


void UEUW_Windows::CloseAllTabs()
{
    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    if (!AssetEditorSubsystem) return;
    
    TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
    
    for (UObject* Asset : EditedAssets)
    {
        if (Asset)
        {
            AssetEditorSubsystem->CloseAllEditorsForAsset(Asset);
        }
    }
    RefreshTabs();
}

void UEUW_Windows::SaveAllDirtyAssets()
{
    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    if (!AssetEditorSubsystem) return;
    
    TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
    TArray<UPackage*> PackagesToSave;
    
    for (UObject* Asset : EditedAssets)
    {
        if (Asset && Asset->GetPackage()->IsDirty())
        {
            PackagesToSave.Add(Asset->GetPackage());
        }
    }
    
    if (PackagesToSave.Num() > 0)
    {
        FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
        RefreshTabs();
    }
}

void UEUW_Windows::RefreshTabs()
{
    InternalRefresh();
    OnTabsChanged.Broadcast();
}

void UEUW_Windows::StartAutoRefresh(float IntervalSeconds)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            AutoRefreshTimerHandle,
            FTimerDelegate::CreateUObject(this, &UEUW_Windows::InternalRefresh),
            IntervalSeconds, true
        );
    }
}

void UEUW_Windows::StopAutoRefresh()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoRefreshTimerHandle);
    }
}

void UEUW_Windows::InternalRefresh()
{
    GetAllOpenTabs();
}

TMap<FString, FLinearColor> UEUW_Windows::GetGroupColorMap()
{
    return GroupColors;
}

FLinearColor UEUW_Windows::ParseHexColor(const FString& HexColor)
{
    if (HexColor.StartsWith(TEXT("#")))
    {
        return FLinearColor(FColor::FromHex(HexColor.RightChop(1)));
    }
    return FLinearColor::White;
}

FString UEUW_Windows::GetAssetTypeDisplayName(UClass* AssetClass)
{
    if (!AssetClass) return TEXT("其他");
    
    FString ClassName = AssetClass->GetName();
    
    static TMap<FString, FString> TypeNameMap = {
        {TEXT("Blueprint"), TEXT("蓝图")},
        {TEXT("WidgetBlueprint"), TEXT("控件蓝图")},
        {TEXT("AnimBlueprint"), TEXT("动画蓝图")},
        {TEXT("Material"), TEXT("材质")},
        {TEXT("MaterialInstanceConstant"), TEXT("材质实例")},
        {TEXT("Texture2D"), TEXT("纹理")},
        {TEXT("StaticMesh"), TEXT("静态网格体")},
        {TEXT("SkeletalMesh"), TEXT("骨骼网格体")},
        {TEXT("AnimSequence"), TEXT("动画序列")},
        {TEXT("AnimMontage"), TEXT("动画蒙太奇")},
        {TEXT("SoundWave"), TEXT("音频")},
        {TEXT("SoundCue"), TEXT("音频")},
        {TEXT("NiagaraSystem"), TEXT("Niagara系统")},
        {TEXT("World"), TEXT("关卡")},
        {TEXT("DataTable"), TEXT("数据表")},
        {TEXT("CurveFloat"), TEXT("曲线")},
        {TEXT("EditorUtilityWidgetBlueprint"), TEXT("控件蓝图")},
    };
    
    if (TypeNameMap.Contains(ClassName)) return TypeNameMap[ClassName];
    return TEXT("其他");
}

FLinearColor UEUW_Windows::GetAssetTypeColor(const FString& AssetType)
{
    if (GroupColors.Contains(AssetType)) return GroupColors[AssetType];
    return GroupColors[TEXT("其他")];
}