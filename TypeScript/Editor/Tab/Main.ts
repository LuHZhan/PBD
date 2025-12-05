/**
 * Main.ts - PuerTS 入口文件
 *
 * 由 C++ 调用 JsEnv->Start("TypeScript/Editor/Tab/Main") 启动
 *
 * 启动流程：
 * 1. C++ JsEnv->Start() 调用此文件
 * 2. 此文件 import 各个 Mixin 模块
 * 3. 模块加载时自动执行 blueprint.mixin()
 * 4. Mixin 生效，蓝图实例获得 TS 方法
 */

import { argv } from 'puerts';

// ============================================================
// 导入 Mixin 模块（导入时自动执行 mixin）
// ============================================================

// 先导入基础模块
import './Types';
import './WidgetHelper';
import './EditorTabService';

// 再导入 Mixin（有依赖顺序）
import { WBP_TabItemWithMixin } from './TabItemMixin';
import { WBP_TabGroupWithMixin } from './TabGroupMixin';
import { EDU_OpenedEditorWithMixin } from './TabManagerMixin';

// 导入调试工具
import { printAllRegistrations } from './WidgetHelper';

// ============================================================
// 初始化
// ============================================================

console.log('========================================');
console.log('[VerticalTabs] TypeScript 模块加载完成');
console.log('========================================');

// 打印组件注册信息（调试用）
printAllRegistrations();

// ============================================================
// 获取 C++ 传入的参数（可选）
// ============================================================

// 示例：获取 C++ 传入的 GameMode
// const gameMode = argv.getByName("GameMode");

// ============================================================
// 导出供外部使用
// ============================================================

export {
    WBP_TabItemWithMixin,
    WBP_TabGroupWithMixin,
    EDU_OpenedEditorWithMixin
};