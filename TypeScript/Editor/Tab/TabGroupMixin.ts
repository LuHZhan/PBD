import * as UE from 'ue';
import { blueprint } from 'puerts';
import { IEditorTabInfo, ITabGroupData } from './Types';
import { WBP_TabItemWithMixin, TabItemWidget } from './TabItemMixin';

// ============================================================
// 1. 加载蓝图类
// ============================================================
const tabGroupCls = UE.Class.Load('/VerticalWindows/Editor/WBP_TabGroup.WBP_TabGroup_C');
const WBP_TabGroup = blueprint.tojs<any>(tabGroupCls);

// ============================================================
// 2. interface 扩展
// ============================================================
interface TabGroupMixin extends UE.UserWidget {}

// ============================================================
// 3. Mixin 类定义
// ============================================================
class TabGroupMixin {
    // WeakMap 存储数据
    private static _dataMap = new WeakMap<object, ITabGroupData>();
    private static _tabsMap = new WeakMap<object, Map<string, TabItemWidget>>();
    private static _callbackMap = new WeakMap<object, {
        onToggle?: (group: ITabGroupData, expanded: boolean) => void;
        onTabClick?: (tab: IEditorTabInfo) => void;
        onTabClose?: (tab: IEditorTabInfo) => void;
    }>();

    // ============ 数据方法 ============

    /**
     * 设置分组数据
     */
    setGroupData(data: ITabGroupData): void {
        TabGroupMixin._dataMap.set(this, data);
        TabGroupMixin._tabsMap.set(this, new Map());
        this._updateView();
        this._rebuildTabs();
    }

    /**
     * 获取分组数据
     */
    getGroupData(): ITabGroupData | undefined {
        return TabGroupMixin._dataMap.get(this);
    }

    /**
     * 设置回调
     */
    setCallbacks(callbacks: {
        onToggle?: (group: ITabGroupData, expanded: boolean) => void;
        onTabClick?: (tab: IEditorTabInfo) => void;
        onTabClose?: (tab: IEditorTabInfo) => void;
    }): void {
        TabGroupMixin._callbackMap.set(this, callbacks);
    }

    // ============ Tab 操作 ============

    /**
     * 重建所有标签
     */
    private _rebuildTabs(): void {
        const data = this.getGroupData();
        if (!data) return;

        const itemContainer = this.GetWidgetFromName("ItemContainer") as UE.VerticalBox;
        if (!itemContainer) {
            console.warn('[TabGroup] ItemContainer not found');
            return;
        }

        // 清空
        itemContainer.ClearChildren();
        const tabsMap = TabGroupMixin._tabsMap.get(this);
        if (tabsMap) tabsMap.clear();

        const callbacks = TabGroupMixin._callbackMap.get(this);

        // 创建子项
        for (const tabData of data.tabs) {
            this._createTabWidget(tabData, itemContainer, callbacks);
        }
    }

    /**
     * 创建单个标签 Widget
     */
    private _createTabWidget(
        tabData: IEditorTabInfo,
        container: UE.VerticalBox,
        callbacks?: {
            onTabClick?: (tab: IEditorTabInfo) => void;
            onTabClose?: (tab: IEditorTabInfo) => void;
        }
    ): void {
        const tabsMap = TabGroupMixin._tabsMap.get(this);
        if (!tabsMap) return;

        // 创建实例
        const tabWidget = UE.WidgetBlueprintLibrary.Create(
            this,
            WBP_TabItemWithMixin.StaticClass(),
            null
        ) as TabItemWidget;

        // 设置数据和回调
        tabWidget.setTabData(tabData);
        tabWidget.setCallbacks(
            (tab) => {
                if (callbacks?.onTabClick) callbacks.onTabClick(tab);
            },
            (tab) => {
                this.removeTab(tab.tabId);
                if (callbacks?.onTabClose) callbacks.onTabClose(tab);
            }
        );

        // 添加到容器
        container.AddChildToVerticalBox(tabWidget);
        tabsMap.set(tabData.tabId, tabWidget);
    }

    /**
     * 添加标签
     */
    addTab(tab: IEditorTabInfo): void {
        const data = this.getGroupData();
        const tabsMap = TabGroupMixin._tabsMap.get(this);
        if (!data || !tabsMap) return;

        // 检查重复
        if (tabsMap.has(tab.tabId)) return;

        data.tabs.push(tab);

        const itemContainer = this.GetWidgetFromName("ItemContainer") as UE.VerticalBox;
        const callbacks = TabGroupMixin._callbackMap.get(this);

        if (itemContainer) {
            this._createTabWidget(tab, itemContainer, callbacks);
        }

        this._updateCount();
    }

    /**
     * 移除标签
     */
    removeTab(tabId: string): void {
        const data = this.getGroupData();
        const tabsMap = TabGroupMixin._tabsMap.get(this);
        if (!data || !tabsMap) return;

        // 从数据中移除
        const idx = data.tabs.findIndex(t => t.tabId === tabId);
        if (idx >= 0) {
            data.tabs.splice(idx, 1);
        }

        // 移除 Widget
        const widget = tabsMap.get(tabId);
        if (widget) {
            widget.RemoveFromParent();
            tabsMap.delete(tabId);
        }

        this._updateCount();
    }

    /**
     * 清空所有标签
     */
    clearTabs(): void {
        const data = this.getGroupData();
        const tabsMap = TabGroupMixin._tabsMap.get(this);

        if (data) data.tabs = [];

        const itemContainer = this.GetWidgetFromName("ItemContainer") as UE.VerticalBox;
        if (itemContainer) itemContainer.ClearChildren();

        if (tabsMap) tabsMap.clear();

        this._updateCount();
    }

    // ============ 展开/折叠 ============

    /**
     * 设置展开状态
     */
    setExpanded(expanded: boolean): void {
        const data = this.getGroupData();
        if (!data) return;

        data.expanded = expanded;

        // 更新容器可见性
        const itemContainer = this.GetWidgetFromName("ItemContainer") as UE.VerticalBox;
        if (itemContainer) {
            itemContainer.SetVisibility(
                expanded ? UE.ESlateVisibility.Visible : UE.ESlateVisibility.Collapsed
            );
        }

        // 更新箭头图标
        this._updateExpandIcon();

        // 触发回调
        const callbacks = TabGroupMixin._callbackMap.get(this);
        if (callbacks?.onToggle) {
            callbacks.onToggle(data, expanded);
        }
    }

    /**
     * 获取展开状态
     */
    isExpanded(): boolean {
        return this.getGroupData()?.expanded ?? true;
    }

    /**
     * 获取标签数量
     */
    getTabCount(): number {
        return this.getGroupData()?.tabs.length ?? 0;
    }

    // ============ UI 更新 ============

    /**
     * 更新视图
     */
    private _updateView(): void {
        const data = this.getGroupData();
        if (!data) return;

        // 分组名
        const groupNameText = this.GetWidgetFromName("GroupNameText") as UE.TextBlock;
        if (groupNameText) {
            groupNameText.SetText(data.groupName);
        }

        // 颜色条
        const colorBar = this.GetWidgetFromName("ColorBar") as UE.Image;
        if (colorBar && data.color) {
            colorBar.SetColorAndOpacity(data.color);
        }

        // 容器可见性
        const itemContainer = this.GetWidgetFromName("ItemContainer") as UE.VerticalBox;
        if (itemContainer) {
            itemContainer.SetVisibility(
                data.expanded ? UE.ESlateVisibility.Visible : UE.ESlateVisibility.Collapsed
            );
        }

        // 展开图标
        this._updateExpandIcon();

        // 计数
        this._updateCount();
    }

    /**
     * 更新展开图标
     */
    private _updateExpandIcon(): void {
        const data = this.getGroupData();
        const expandIcon = this.GetWidgetFromName("ExpandIcon") as UE.Image;

        if (expandIcon && data) {
            const transform = new UE.WidgetTransform();
            transform.Angle = data.expanded ? 90 : 0;
            expandIcon.SetRenderTransform(transform);
        }
    }

    /**
     * 更新计数显示
     */
    private _updateCount(): void {
        const countText = this.GetWidgetFromName("CountText") as UE.TextBlock;
        if (countText) {
            countText.SetText(`(${this.getTabCount()})`);
        }
    }

    // ============ 蓝图事件 ============

    /**
     * 头部点击 - 在蓝图中将 HeaderButton.OnClicked 连接到此方法
     */
    OnHeaderClicked(): void {
        this.setExpanded(!this.isExpanded());
    }
}

// ============================================================
// 4. 执行 Mixin
// ============================================================
export const WBP_TabGroupWithMixin = blueprint.mixin(WBP_TabGroup, TabGroupMixin);

// ============================================================
// 5. 导出类型
// ============================================================
export type TabGroupWidget = InstanceType<typeof WBP_TabGroupWithMixin>;