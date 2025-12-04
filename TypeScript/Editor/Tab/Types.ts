import * as UE from 'ue';

/**
 * 编辑器标签页信息
 */
export interface IEditorTabInfo {
    tabId: string;
    displayName: string;
    assetPath: string;
    assetType: string;
    assetClassName: string;
    isDirty: boolean;
    isActive: boolean;
    groupId: string;
    groupColor: UE.LinearColor;
}

/**
 * 分组数据
 */
export interface ITabGroupData {
    groupId: string;
    groupName: string;
    color: UE.LinearColor;
    expanded: boolean;
    tabs: IEditorTabInfo[];
}

/**
 * 回调类型
 */
export type OnTabClickCallback = (tab: IEditorTabInfo) => void;
export type OnTabCloseCallback = (tab: IEditorTabInfo) => void;
export type OnGroupToggleCallback = (group: ITabGroupData, expanded: boolean) => void;