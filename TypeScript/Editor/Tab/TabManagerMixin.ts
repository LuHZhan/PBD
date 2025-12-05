import * as UE from 'ue';
import { blueprint } from 'puerts';
import { IEditorTabInfo, ITabGroupData } from './Types';
import { EditorTabService } from './EditorTabService';
import { WBP_TabGroupWithMixin, TabGroupWidget } from './TabGroupMixin';
import {
    registerWidgetComponents,
    createWidgetAccessor,
    required,
    optional
} from './WidgetHelper';

// ============================================================
// 配置
// ============================================================

const CLASS_NAME = 'EDU_OpenedEditor';
const BLUEPRINT_PATH = '/VerticalWindows/Editor/EDU_OpenedEditor';

// ============================================================
// 1. 注册组件需求
// ============================================================
registerWidgetComponents(CLASS_NAME, BLUEPRINT_PATH, [
    required('GroupContainer',    'VerticalBox', '分组列表容器'),
    optional('RefreshButton',     'Button',      '刷新按钮'),
    optional('ExpandAllButton',   'Button',      '全部展开按钮'),
    optional('CollapseAllButton', 'Button',      '全部折叠按钮'),
    optional('SaveAllButton',     'Button',      '全部保存按钮'),
    optional('SearchBox',         'EditableText','搜索框'),
]);

// ============================================================
// 2. 加载蓝图类
// ============================================================
const euwCls = UE.Class.Load(`${BLUEPRINT_PATH}.${CLASS_NAME}_C`);
if (!euwCls) {
    console.error(`[${CLASS_NAME}] 无法加载蓝图类: ${BLUEPRINT_PATH}.${CLASS_NAME}_C`);
    console.error(`请确保蓝图文件存在于正确路径`);
}
const EDU_OpenedEditor = euwCls ? blueprint.tojs<any>(euwCls) : null;

// ============================================================
// 3. Mixin 类定义
// ============================================================
class TabManagerMixin {
    // WeakMap 存储数据
    private static _serviceMap = new WeakMap<object, EditorTabService>();
    private static _groupsMap = new WeakMap<object, Map<string, any>>();
    private static _expandStatesMap = new WeakMap<object, Map<string, boolean>>();
    private static _initializedMap = new WeakMap<object, boolean>();
    private static _accessorMap = new WeakMap<object, ReturnType<typeof createWidgetAccessor>>();

    // ============ 辅助方法 ============

    private _accessor() {
        let accessor = TabManagerMixin._accessorMap.get(this);
        if (!accessor) {
            accessor = createWidgetAccessor(this, CLASS_NAME);
            TabManagerMixin._accessorMap.set(this, accessor);
        }
        return accessor;
    }

    private _self(): any {
        return this;
    }

    // ============ 初始化 ============

    /**
     * 初始化标签管理器 - 在蓝图 Construct 中调用此方法
     */
    initTabManager(): void {
        // 防止重复初始化
        if (TabManagerMixin._initializedMap.get(this)) {
            console.warn('[TabManager] Already initialized');
            return;
        }

        // 初始化数据存储
        TabManagerMixin._serviceMap.set(this, new EditorTabService(this._self()));
        TabManagerMixin._groupsMap.set(this, new Map());
        TabManagerMixin._expandStatesMap.set(this, new Map());
        TabManagerMixin._initializedMap.set(this, true);

        // 绑定 C++ 刷新事件
        const self = this._self();
        if (self.OnTabsChanged) {
            self.OnTabsChanged.Add(() => {
                this.syncWithEditor();
            });
        }

        // 初始同步
        this.syncWithEditor();

        console.log('[VerticalTabs] TabManager initialized');
    }

    /**
     * 清理 - 在蓝图 Destruct 中调用
     */
    cleanupTabManager(): void {
        TabManagerMixin._serviceMap.delete(this);
        TabManagerMixin._groupsMap.delete(this);
        TabManagerMixin._expandStatesMap.delete(this);
        TabManagerMixin._initializedMap.delete(this);
        TabManagerMixin._accessorMap.delete(this);

        console.log('[VerticalTabs] TabManager cleaned up');
    }

    // ============ 同步 ============

    /**
     * 与编辑器同步
     */
    syncWithEditor(): void {
        const service = TabManagerMixin._serviceMap.get(this);
        const groupsMap = TabManagerMixin._groupsMap.get(this);
        const expandStates = TabManagerMixin._expandStatesMap.get(this);

        if (!service || !groupsMap || !expandStates) {
            console.warn('[TabManager] Not initialized');
            return;
        }

        // 获取分组数据
        const groups = service.getGroupedTabs();

        // 获取容器
        const groupContainer = this._accessor().getSilent('GroupContainer');
        if (!groupContainer) {
            console.warn('[TabManager] GroupContainer not found');
            return;
        }

        // 清空现有内容
        groupContainer.ClearChildren();
        groupsMap.clear();

        // 重建分组
        for (const groupData of groups) {
            // 恢复之前的展开状态
            if (expandStates.has(groupData.groupId)) {
                groupData.expanded = expandStates.get(groupData.groupId)!;
            }

            // 创建分组 Widget
            const groupWidget = UE.WidgetBlueprintLibrary.Create(
                this._self(),
                WBP_TabGroupWithMixin.StaticClass(),
                null
            ) as any;

            // 设置数据
            groupWidget.setGroupData(groupData);

            // 设置回调
            groupWidget.setCallbacks({
                onToggle: (g: ITabGroupData, exp: boolean) => {
                    // 记住展开状态
                    expandStates.set(g.groupId, exp);
                },
                onTabClick: (tab: IEditorTabInfo) => {
                    // 激活标签
                    service.activateTab(tab.tabId);
                    console.log(`[VerticalTabs] Activated: ${tab.displayName}`);
                },
                onTabClose: (tab: IEditorTabInfo) => {
                    // 关闭标签
                    service.closeTab(tab.tabId);
                    console.log(`[VerticalTabs] Closed: ${tab.displayName}`);
                    // 延迟刷新
                    setTimeout(() => this.syncWithEditor(), 100);
                }
            });

            // 添加到容器
            groupContainer.AddChildToVerticalBox(groupWidget);
            groupsMap.set(groupData.groupId, groupWidget);
        }

        console.log(`[VerticalTabs] Synced ${groups.length} groups`);
    }

    /**
     * 刷新
     */
    refresh(): void {
        this.syncWithEditor();
    }

    // ============ 批量操作 ============

    /**
     * 展开所有分组
     */
    expandAll(): void {
        const groupsMap = TabManagerMixin._groupsMap.get(this);
        const expandStates = TabManagerMixin._expandStatesMap.get(this);
        if (!groupsMap || !expandStates) return;

        for (const [id, group] of groupsMap) {
            group.setExpanded(true);
            expandStates.set(id, true);
        }
    }

    /**
     * 折叠所有分组
     */
    collapseAll(): void {
        const groupsMap = TabManagerMixin._groupsMap.get(this);
        const expandStates = TabManagerMixin._expandStatesMap.get(this);
        if (!groupsMap || !expandStates) return;

        for (const [id, group] of groupsMap) {
            group.setExpanded(false);
            expandStates.set(id, false);
        }
    }

    /**
     * 保存所有修改
     */
    saveAll(): void {
        const self = this._self();
        if (self.SaveAllDirtyAssets_EditorOnly) {
            self.SaveAllDirtyAssets_EditorOnly();
        }
        this.refresh();
    }

    // ============ 蓝图事件处理 ============

    /**
     * 刷新按钮点击 - 在蓝图中连接 RefreshButton.OnClicked
     */
    OnRefreshClicked(): void {
        this.refresh();
    }

    /**
     * 全部展开按钮点击
     */
    OnExpandAllClicked(): void {
        this.expandAll();
    }

    /**
     * 全部折叠按钮点击
     */
    OnCollapseAllClicked(): void {
        this.collapseAll();
    }

    /**
     * 保存全部按钮点击
     */
    OnSaveAllClicked(): void {
        this.saveAll();
    }
}

// ============================================================
// 4. 执行 Mixin
// ============================================================
export const EDU_OpenedEditorWithMixin = EDU_OpenedEditor
    ? blueprint.mixin(EDU_OpenedEditor, TabManagerMixin)
    : null;

if (EDU_OpenedEditorWithMixin) {
    console.log(`[${CLASS_NAME}] Mixin 成功`);
} else {
    console.error(`[${CLASS_NAME}] Mixin 失败，请检查蓝图路径`);
}

// ============================================================
// 5. 导出
// ============================================================
export type TabManagerWidget = InstanceType<typeof EDU_OpenedEditorWithMixin>;