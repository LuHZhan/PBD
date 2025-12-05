import * as UE from 'ue';
import { blueprint } from 'puerts';
import { IEditorTabInfo, ITabGroupData } from './Types';
import { WBP_TabItemWithMixin, TabItemWidget } from './TabItemMixin';
import {
    registerWidgetComponents,
    createWidgetAccessor,
    required,
    optional
} from './WidgetHelper';

// ============================================================
// 配置
// ============================================================

const CLASS_NAME = 'WBP_TabGroup';
const BLUEPRINT_PATH = '/VerticalWindows/Editor/WBP_TabGroup';

// ============================================================
// 1. 注册组件需求
// ============================================================
registerWidgetComponents(CLASS_NAME, BLUEPRINT_PATH, [
    required('HeaderButton',   'Button',      '分组头部点击区域'),
    required('GroupNameText',  'TextBlock',   '分组名称'),
    required('ItemContainer',  'VerticalBox', '子标签项容器'),
    required('ColorBar',       'Image',       '颜色条'),
    optional('ExpandIcon',     'Image',       '展开/折叠箭头图标'),
    optional('CountText',      'TextBlock',   '标签数量显示'),
]);

// ============================================================
// 2. 加载蓝图类
// ============================================================
const tabGroupCls = UE.Class.Load(`${BLUEPRINT_PATH}.${CLASS_NAME}_C`);
if (!tabGroupCls) {
    console.error(`[${CLASS_NAME}] 无法加载蓝图类: ${BLUEPRINT_PATH}.${CLASS_NAME}_C`);
    console.error(`请确保蓝图文件存在于正确路径`);
}
const WBP_TabGroup = tabGroupCls ? blueprint.tojs<any>(tabGroupCls) : null;

// ============================================================
// 3. Mixin 类定义
// ============================================================
class TabGroupMixin {
    // WeakMap 存储数据
    private static _dataMap = new WeakMap<object, ITabGroupData>();
    private static _tabsMap = new WeakMap<object, Map<string, any>>();
    private static _callbackMap = new WeakMap<object, {
        onToggle?: (group: ITabGroupData, expanded: boolean) => void;
        onTabClick?: (tab: IEditorTabInfo) => void;
        onTabClose?: (tab: IEditorTabInfo) => void;
    }>();
    private static _accessorMap = new WeakMap<object, ReturnType<typeof createWidgetAccessor>>();

    // ============ 辅助方法 ============

    private _accessor() {
        let accessor = TabGroupMixin._accessorMap.get(this);
        if (!accessor) {
            accessor = createWidgetAccessor(this, CLASS_NAME);
            TabGroupMixin._accessorMap.set(this, accessor);
        }
        return accessor;
    }

    private _self(): any {
        return this;
    }

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

        const itemContainer = this._accessor().getSilent('ItemContainer');
        if (!itemContainer) return;

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
        container: any,
        callbacks?: {
            onTabClick?: (tab: IEditorTabInfo) => void;
            onTabClose?: (tab: IEditorTabInfo) => void;
        }
    ): void {
        const tabsMap = TabGroupMixin._tabsMap.get(this);
        if (!tabsMap) return;

        // 创建实例
        const tabWidget = UE.WidgetBlueprintLibrary.Create(
            this._self(),
            WBP_TabItemWithMixin.StaticClass(),
            null
        ) as any;

        // 设置数据和回调
        tabWidget.setTabData(tabData);
        tabWidget.setCallbacks(
            (tab: IEditorTabInfo) => {
                if (callbacks?.onTabClick) callbacks.onTabClick(tab);
            },
            (tab: IEditorTabInfo) => {
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

        const itemContainer = this._accessor().getSilent('ItemContainer');
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

        const itemContainer = this._accessor().getSilent('ItemContainer');
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

        const $ = this._accessor();

        // 更新容器可见性
        const itemContainer = $.getSilent('ItemContainer');
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

        const $ = this._accessor();

        // 分组名
        const groupNameText = $.getSilent('GroupNameText');
        if (groupNameText) {
            groupNameText.SetText(data.groupName);
        }

        // 颜色条
        const colorBar = $.getSilent('ColorBar');
        if (colorBar && data.color) {
            colorBar.SetColorAndOpacity(data.color);
        }

        // 容器可见性
        const itemContainer = $.getSilent('ItemContainer');
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
        const expandIcon = this._accessor().getSilent('ExpandIcon');

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
        const countText = this._accessor().getSilent('CountText');
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
export const WBP_TabGroupWithMixin = WBP_TabGroup
    ? blueprint.mixin(WBP_TabGroup, TabGroupMixin)
    : null;

if (WBP_TabGroupWithMixin) {
    console.log(`[${CLASS_NAME}] Mixin 成功`);
} else {
    console.error(`[${CLASS_NAME}] Mixin 失败，请检查蓝图路径`);
}

// ============================================================
// 5. 导出
// ============================================================
export type TabGroupWidget = InstanceType<typeof WBP_TabGroupWithMixin>;