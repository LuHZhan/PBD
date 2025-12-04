/**
 * VerticalWindows - 垂直标签页管理器
 *
 * 使用方法：
 * 1. 在 UE 编辑器中创建以下 Widget 蓝图：
 *    - /VerticalWindows/Editor/WBP_TabItem.uasset
 *    - /VerticalWindows/Editor/WBP_TabGroup.uasset
 *    - /VerticalWindows/Editor/EDU_OpenedEditor.uasset (继承 EUW_Windows)
 *
 * 2. 在 EDU_OpenedEditor 蓝图中：
 *    - Event Construct -> 调用 TS 函数 initTabManager()
 *    - Event Destruct  -> 调用 TS 函数 cleanupTabManager()
 *    - RefreshButton.OnClicked -> 调用 TS 函数 OnRefreshClicked()
 *    - ExpandAllButton.OnClicked -> 调用 TS 函数 OnExpandAllClicked()
 *    - CollapseAllButton.OnClicked -> 调用 TS 函数 OnCollapseAllClicked()
 *    - SaveAllButton.OnClicked -> 调用 TS 函数 OnSaveAllClicked()
 *
 * 3. 在 WBP_TabItem 蓝图中：
 *    - RootButton.OnClicked -> 调用 TS 函数 OnItemClicked()
 *    - CloseButton.OnClicked -> 调用 TS 函数 OnCloseClicked()
 *
 * 4. 在 WBP_TabGroup 蓝图中：
 *    - HeaderButton.OnClicked -> 调用 TS 函数 OnHeaderClicked()
 */

// 导出类型
export * from './Types';

// 导出服务
export { EditorTabService } from './EditorTabService';

// 导出 Mixin 后的类
export { WBP_TabItemWithMixin, TabItemWidget } from './TabItemMixin';
export { WBP_TabGroupWithMixin, TabGroupWidget } from './TabGroupMixin';
export { EDU_OpenedEditorWithMixin, TabManagerWidget } from './TabManagerMixin';

// ============================================================
// 注意：Mixin 在模块加载时自动执行
// 导入此模块后，所有蓝图实例将自动具有 Mixin 中定义的方法
// ============================================================