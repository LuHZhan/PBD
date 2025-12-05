/**
 * VerticalWindows - 垂直标签页管理器
 *
 * ============================================================
 * 使用步骤
 * ============================================================
 *
 * 1. 创建蓝图 Widget：
 *    - /VerticalWindows/Editor/WBP_TabItem.uasset
 *    - /VerticalWindows/Editor/WBP_TabGroup.uasset
 *    - /VerticalWindows/Editor/EDU_OpenedEditor.uasset (继承 EUW_Windows)
 *
 * 2. 在蓝图中为组件勾选 "Is Variable"：
 *
 *    WBP_TabItem 需要勾选：
 *    - RootButton (Button) [必须] - 整个标签项的点击区域
 *    - TitleText (TextBlock) [必须] - 显示标签名称
 *    - ColorBar (Image) [必须] - 左侧颜色条
 *    - Background (Border) [必须] - 背景
 *    - DirtyIndicator (TextBlock) [可选] - 脏标记
 *    - CloseButton (Button) [可选] - 关闭按钮
 *
 *    WBP_TabGroup 需要勾选：
 *    - HeaderButton (Button) [必须] - 分组头部点击区域
 *    - GroupNameText (TextBlock) [必须] - 分组名称
 *    - ItemContainer (VerticalBox) [必须] - 子标签项容器
 *    - ColorBar (Image) [必须] - 颜色条
 *    - ExpandIcon (Image) [可选] - 展开/折叠箭头
 *    - CountText (TextBlock) [可选] - 标签数量
 *
 *    EDU_OpenedEditor 需要勾选：
 *    - GroupContainer (VerticalBox) [必须] - 分组列表容器
 *    - RefreshButton (Button) [可选] - 刷新按钮
 *    - ExpandAllButton (Button) [可选] - 全部展开按钮
 *    - CollapseAllButton (Button) [可选] - 全部折叠按钮
 *    - SaveAllButton (Button) [可选] - 全部保存按钮
 *
 * 3. 在蓝图中连接事件：
 *
 *    EDU_OpenedEditor:
 *    - Event Construct -> initTabManager()
 *    - Event Destruct -> cleanupTabManager()
 *    - RefreshButton.OnClicked -> OnRefreshClicked()
 *
 *    WBP_TabItem:
 *    - RootButton.OnClicked -> OnItemClicked()
 *    - CloseButton.OnClicked -> OnCloseClicked()
 *
 *    WBP_TabGroup:
 *    - HeaderButton.OnClicked -> OnHeaderClicked()
 *
 * ============================================================
 * 调试
 * ============================================================
 *
 * 如果组件未正确勾选 "Is Variable"，运行时会输出类似：
 *
 * ========== [WBP_TabItem] 组件检查 ==========
 * 蓝图路径: /VerticalWindows/Editor/WBP_TabItem
 * 已找到 2/7 个组件
 *
 * ❌ 缺失必须组件 (2个):
 *    请在蓝图中为以下组件勾选 "Is Variable":
 *
 *    • TitleText: TextBlock - 显示标签名称
 *    • RootButton: Button - 整个标签项的点击区域
 *
 * ============================================
 *
 * 可调用 printAllRegistrations() 查看所有注册的组件需求。
 */

// ============================================================
// 导出
// ============================================================

// 类型
export * from './Types';

// 工具
export {
    registerWidgetComponents,
    createWidgetAccessor,
    validateWidget,
    required,
    optional,
    printAllRegistrations,
    getRegistrations
} from './WidgetHelper';

// 服务
export { EditorTabService } from './EditorTabService';

// Mixin 类
export { WBP_TabItemWithMixin, TabItemWidget } from './TabItemMixin';
export { WBP_TabGroupWithMixin, TabGroupWidget } from './TabGroupMixin';
export { EDU_OpenedEditorWithMixin, TabManagerWidget } from './TabManagerMixin';

// ============================================================
// 注意：Mixin 在模块加载时自动执行
// 导入此模块后，所有蓝图实例将自动具有 Mixin 中定义的方法
// ============================================================