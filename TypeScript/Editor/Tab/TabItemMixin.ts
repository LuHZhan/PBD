import * as UE from 'ue';
import { blueprint } from 'puerts';
import { IEditorTabInfo } from './Types';

// ============================================================
// 1. 加载蓝图类（保持引用防止被 GC）
// ============================================================
const tabItemCls = UE.Class.Load('/VerticalWindows/Editor/WBP_TabItem.WBP_TabItem_C');
const WBP_TabItem = blueprint.tojs<any>(tabItemCls);

// ============================================================
// 2. 声明 interface 让 Mixin 能访问 Widget 方法
// ============================================================
interface TabItemMixin extends UE.UserWidget {}

// ============================================================
// 3. Mixin 类定义
// ============================================================
class TabItemMixin {
    // 使用 WeakMap 存储数据，避免内存泄漏
    private static _dataMap = new WeakMap<object, IEditorTabInfo>();
    private static _callbackMap = new WeakMap<object, {
        onClick?: (tab: IEditorTabInfo) => void;
        onClose?: (tab: IEditorTabInfo) => void;
    }>();

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

        const titleText = this.GetWidgetFromName("TitleText") as UE.TextBlock;
        const colorBar = this.GetWidgetFromName("ColorBar") as UE.Image;
        const dirtyIndicator = this.GetWidgetFromName("DirtyIndicator") as UE.TextBlock;
        const rootButton = this.GetWidgetFromName("RootButton") as UE.Button;

        if (titleText) {
            titleText.SetText(data.displayName);
        }

        if (colorBar && data.groupColor) {
            colorBar.SetColorAndOpacity(data.groupColor);
        }

        if (dirtyIndicator) {
            dirtyIndicator.SetVisibility(
                data.isDirty ? UE.ESlateVisibility.Visible : UE.ESlateVisibility.Collapsed
            );
        }

        if (rootButton) {
            rootButton.SetToolTipText(`${data.assetPath}\n类型: ${data.assetType}`);
        }
    }

    /**
     * 设置选中状态
     */
    setSelected(selected: boolean): void {
        const background = this.GetWidgetFromName("Background") as UE.Border;
        if (background) {
            const color = selected
                ? new UE.LinearColor(0.2, 0.4, 0.8, 0.5)
                : new UE.LinearColor(0, 0, 0, 0);
            background.SetBrushColor(color);
        }
    }

    // ============ 事件处理（蓝图中连接按钮到这些方法） ============

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
// 5. 导出类型
// ============================================================
export type TabItemWidget = InstanceType<typeof WBP_TabItemWithMixin>;