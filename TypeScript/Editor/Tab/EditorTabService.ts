import * as UE from 'ue';
import { IEditorTabInfo, ITabGroupData } from './Types';

/**
 * 编辑器标签页服务
 * 封装与 C++ UEUW_Windows 的交互
 */
export class EditorTabService {
    private widget: any;

    constructor(widget: any) {
        this.widget = widget;
    }

    /**
     * 获取所有打开的标签页
     */
    getAllOpenTabs(): IEditorTabInfo[] {
        const ueTabs = this.widget.GetAllOpenTabs_EditorOnly();
        return this.convertTabs(ueTabs);
    }

    /**
     * 获取按类型分组的标签页
     */
    getGroupedTabs(): ITabGroupData[] {
        const ueGroups = this.widget.GetGroupedTabs_EditorOnly();
        const result: ITabGroupData[] = [];

        if (!ueGroups) return result;

        for (let i = 0; i < ueGroups.Num(); i++) {
            const g = ueGroups.Get(i);
            result.push({
                groupId: g.GroupId,
                groupName: g.GroupName,
                color: g.Color,
                expanded: g.bExpanded,
                tabs: this.convertTabs(g.Tabs)
            });
        }
        return result;
    }

    /**
     * 激活标签页
     */
    activateTab(tabId: string): boolean {
        return this.widget.ActivateTab_EditorOnly(tabId);
    }

    /**
     * 关闭标签页
     */
    closeTab(tabId: string): boolean {
        return this.widget.CloseTab_EditorOnly(tabId);
    }

    /**
     * 在内容浏览器中定位资产
     */
    browseToAsset(assetPath: string): void {
        this.widget.BrowseToAsset_EditorOnly(assetPath);
    }

    /**
     * 保存资产
     */
    saveAsset(assetPath: string): boolean {
        return this.widget.SaveAsset_EditorOnly(assetPath);
    }

    /**
     * 保存所有脏资产
     */
    saveAllDirty(): void {
        this.widget.SaveAllDirtyAssets_EditorOnly();
    }

    /**
     * 刷新
     */
    refresh(): void {
        this.widget.RefreshTabs_EditorOnly();
    }

    /**
     * 转换 UE TArray 到 JS 数组
     */
    private convertTabs(ueTabs: any): IEditorTabInfo[] {
        const result: IEditorTabInfo[] = [];
        if (!ueTabs) return result;

        for (let i = 0; i < ueTabs.Num(); i++) {
            const t = ueTabs.Get(i);
            result.push({
                tabId: t.TabId,
                displayName: t.DisplayName,
                assetPath: t.AssetPath,
                assetType: t.AssetType,
                assetClassName: t.AssetClassName,
                isDirty: t.bIsDirty,
                isActive: t.bIsActive,
                groupId: t.GroupId,
                groupColor: t.GroupColor
            });
        }
        return result;
    }
}