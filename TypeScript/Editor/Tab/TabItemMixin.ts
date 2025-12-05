import * as UE from 'ue';
import { blueprint } from 'puerts';
import { IEditorTabInfo } from './Types';
import {
    registerWidgetComponents,
    createWidgetAccessor,
    required,
    optional
} from './WidgetHelper';

// ============================================================
// 配置
// ============================================================

const CLASS_NAME = 'WBP_TabItem';
const BLUEPRINT_PATH = '/VerticalWindows/Editor/Components/WBP_TabItem';

// ============================================================
// 1. 注册组件需求
// ============================================================
registerWidgetComponents(CLASS_NAME, BLUEPRINT_PATH, [
    required('RootButton',     'Button',    '整个标签项的点击区域'),
    required('TitleText',      'TextBlock', '显示标签名称'),
    // required('ColorBar',       'Image',     '左侧颜色条，表示资产类型'),
    required('Background',     'Border',    '背景，用于显示选中状态'),
    optional('DirtyIndicator', 'TextBlock', '脏标记 (*)'),
    optional('CloseButton',    'Button',    '关闭按钮'),
    optional('IconImage',      'Image',     '资产图标'),
]);

// ============================================================
// 2. 加载蓝图类
// ============================================================
const tabItemCls = UE.Class.Load(`${BLUEPRINT_PATH}.${CLASS_NAME}_C`);
const WBP_TabItem = blueprint.tojs<any>(tabItemCls);

// ============================================================
// 3. Mixin 类定义
// ============================================================
class TabItemMixin {
    // WeakMap 存储数据
    private static _dataMap = new WeakMap<object, IEditorTabInfo>();
    private static _callbackMap = new WeakMap<object, {
        onClick?: (tab: IEditorTabInfo) => void;
        onClose?: (tab: IEditorTabInfo) => void;
    }>();
    private static _accessorMap = new WeakMap<object, ReturnType<typeof createWidgetAccessor>>();

    // ============ 辅助方法 ============

    /**
     * 获取组件访问器
     */
    private _accessor() {
        let accessor = TabItemMixin._accessorMap.get(this);
        if (!accessor) {
            accessor = createWidgetAccessor(this, CLASS_NAME);
            TabItemMixin._accessorMap.set(this, accessor);
        }
        return accessor;
    }

    // ============ 数据方法 ============

    /**
     * 设置标签数据
     */
    setTabData(data: IEditorTabInfo): void {
        TabItemMixin._dataMap.set(this, data);
        this._updateView();
    }

    /**
     * 获取标签数据
     */
    getTabData(): IEditorTabInfo | undefined {
        return TabItemMixin._dataMap.get(this);
    }

    /**
     * 设置回调
     */
    setCallbacks(
        onClick?: (tab: IEditorTabInfo) => void,
        onClose?: (tab: IEditorTabInfo) => void
    ): void {
        TabItemMixin._callbackMap.set(this, { onClick, onClose });
    }

    // ============ UI 更新 ============

    /**
     * 更新视图
     */
    private _updateView(): void {
        const data = this.getTabData();
        if (!data) return;

        const $ = this._accessor();

        // 标题
        const titleText = $.getSilent('TitleText');
        if (titleText) {
            titleText.SetText(data.displayName);
        }

        // 颜色条
        // const colorBar = $.getSilent('ColorBar');
        // if (colorBar && data.groupColor) {
        //     colorBar.SetColorAndOpacity(data.groupColor);
        // }

        // 脏标记
        const dirtyIndicator = $.getSilent('DirtyIndicator');
        if (dirtyIndicator) {
            dirtyIndicator.SetVisibility(
                data.isDirty ? UE.ESlateVisibility.Visible : UE.ESlateVisibility.Collapsed
            );
        }

        // 工具提示
        const rootButton = $.getSilent('RootButton');
        if (rootButton) {
            rootButton.SetToolTipText(`${data.assetPath}\n类型: ${data.assetType}`);
        }
    }

    /**
     * 设置选中状态
     */
    setSelected(selected: boolean): void {
        const background = this._accessor().getSilent('Background');
        if (background) {
            const color = selected
                ? new UE.LinearColor(0.2, 0.4, 0.8, 0.5)
                : new UE.LinearColor(0, 0, 0, 0);
            background.SetBrushColor(color);
        }
    }

    // ============ 事件处理 ============

    /**
     * 点击标签项 - 在蓝图中将 RootButton.OnClicked 连接到此方法
     */
    OnItemClicked(): void {
        const data = this.getTabData();
        const callbacks = TabItemMixin._callbackMap.get(this);
        if (data && callbacks?.onClick) {
            callbacks.onClick(data);
        }
    }

    /**
     * 点击关闭按钮 - 在蓝图中将 CloseButton.OnClicked 连接到此方法
     */
    OnCloseClicked(): void {
        const data = this.getTabData();
        const callbacks = TabItemMixin._callbackMap.get(this);
        if (data && callbacks?.onClose) {
            callbacks.onClose(data);
        }
    }
}

// ============================================================
// 4. 执行 Mixin
// ============================================================
export const WBP_TabItemWithMixin = blueprint.mixin(WBP_TabItem, TabItemMixin);

// ============================================================
// 5. 导出
// ============================================================
export type TabItemWidget = InstanceType<typeof WBP_TabItemWithMixin>;