/**
 * WidgetHelper.ts
 * 通用的 Widget 组件访问工具
 *
 * 用于管理蓝图组件的 "Is Variable" 需求
 */

// ============================================================
// 类型定义
// ============================================================

/**
 * 单个组件需求
 */
export interface IWidgetComponentRequirement {
    /** 组件名称（蓝图中的变量名） */
    name: string;
    /** 期望的类型（用于提示，如 TextBlock, Button） */
    type: string;
    /** 是否必须 */
    required: boolean;
    /** 描述（用于提示） */
    description?: string;
}

/**
 * Widget 类的组件需求注册信息
 */
interface IWidgetRegistration {
    /** Widget 类名 */
    className: string;
    /** 蓝图路径 */
    blueprintPath: string;
    /** 需要的组件列表 */
    components: IWidgetComponentRequirement[];
}

// ============================================================
// 全局注册表
// ============================================================

/** 存储所有 Widget 类的组件需求 */
const _registrations = new Map<string, IWidgetRegistration>();

/** 已验证过的实例（避免重复输出警告） */
const _validatedInstances = new WeakSet<object>();

// ============================================================
// 注册 API
// ============================================================

/**
 * 注册一个 Widget 类需要的组件
 *
 * @example
 * registerWidgetComponents('WBP_TabItem', '/VerticalWindows/Editor/WBP_TabItem', [
 *     { name: 'TitleText', type: 'TextBlock', required: true, description: '标题文本' },
 *     { name: 'RootButton', type: 'Button', required: true, description: '点击区域' },
 * ]);
 */
export function registerWidgetComponents(
    className: string,
    blueprintPath: string,
    components: IWidgetComponentRequirement[]
): void {
    _registrations.set(className, {
        className,
        blueprintPath,
        components
    });
}

/**
 * 快捷方式：定义必须组件
 */
export function required(name: string, type: string, description?: string): IWidgetComponentRequirement {
    return { name, type, required: true, description };
}

/**
 * 快捷方式：定义可选组件
 */
export function optional(name: string, type: string, description?: string): IWidgetComponentRequirement {
    return { name, type, required: false, description };
}

// ============================================================
// 验证 API
// ============================================================

/**
 * 验证 Widget 实例的所有组件是否可用
 * 会输出缺失组件的详细信息
 *
 * @returns 是否所有必须组件都存在
 */
export function validateWidget(widget: any, className: string): boolean {
    // 避免重复验证
    if (_validatedInstances.has(widget)) {
        return true;
    }
    _validatedInstances.add(widget);

    const registration = _registrations.get(className);
    if (!registration) {
        console.warn(`[WidgetHelper] No registration found for "${className}". Call registerWidgetComponents() first.`);
        return true;
    }

    const missing: IWidgetComponentRequirement[] = [];
    const found: string[] = [];

    for (const comp of registration.components) {
        if (widget[comp.name] !== undefined && widget[comp.name] !== null) {
            found.push(comp.name);
        } else {
            missing.push(comp);
        }
    }

    // 输出结果
    if (missing.length > 0) {
        const requiredMissing = missing.filter(c => c.required);
        const optionalMissing = missing.filter(c => !c.required);

        console.warn(`\n========== [${className}] 组件检查 ==========`);
        console.warn(`蓝图路径: ${registration.blueprintPath}`);
        console.warn(`已找到 ${found.length}/${registration.components.length} 个组件`);

        if (requiredMissing.length > 0) {
            console.error(`\n❌ 缺失必须组件 (${requiredMissing.length}个):`);
            console.error(`   请在蓝图中为以下组件勾选 "Is Variable":\n`);
            for (const comp of requiredMissing) {
                const desc = comp.description ? ` - ${comp.description}` : '';
                console.error(`   • ${comp.name}: ${comp.type}${desc}`);
            }
        }

        if (optionalMissing.length > 0) {
            console.warn(`\n⚠️ 缺失可选组件 (${optionalMissing.length}个):`);
            for (const comp of optionalMissing) {
                const desc = comp.description ? ` - ${comp.description}` : '';
                console.warn(`   • ${comp.name}: ${comp.type}${desc}`);
            }
        }

        console.warn(`\n============================================\n`);

        return requiredMissing.length === 0;
    }

    console.log(`[${className}] ✓ 所有组件检查通过`);
    return true;
}

// ============================================================
// 获取组件 API
// ============================================================

/**
 * 创建一个组件访问器
 * 用于安全地获取蓝图组件，未找到时输出警告
 */
export function createWidgetAccessor(widget: any, className: string) {
    // 首次访问时验证
    validateWidget(widget, className);

    return {
        /**
         * 获取组件，未找到返回 null
         */
        get<T = any>(name: string): T | null {
            const comp = widget[name];
            if (comp === undefined || comp === null) {
                // 只在第一次访问时警告
                console.warn(`[${className}] 组件 "${name}" 未找到，请确保已勾选 "Is Variable"`);
                return null;
            }
            return comp as T;
        },

        /**
         * 获取组件，未找到不输出警告
         */
        getSilent<T = any>(name: string): T | null {
            const comp = widget[name];
            return (comp !== undefined && comp !== null) ? comp as T : null;
        },

        /**
         * 检查组件是否存在
         */
        has(name: string): boolean {
            return widget[name] !== undefined && widget[name] !== null;
        },

        /**
         * 获取原始 widget 引用
         */
        raw(): any {
            return widget;
        }
    };
}

// ============================================================
// 辅助函数
// ============================================================

/**
 * 获取所有已注册的 Widget 信息（调试用）
 */
export function getRegistrations(): Map<string, IWidgetRegistration> {
    return new Map(_registrations);
}

/**
 * 打印所有注册信息（调试用）
 */
export function printAllRegistrations(): void {
    console.log('\n========== Widget 组件注册表 ==========\n');
    for (const [name, reg] of _registrations) {
        console.log(`[${name}]`);
        console.log(`  路径: ${reg.blueprintPath}`);
        console.log(`  组件:`);
        for (const comp of reg.components) {
            const req = comp.required ? '[必须]' : '[可选]';
            const desc = comp.description ? ` - ${comp.description}` : '';
            console.log(`    • ${comp.name}: ${comp.type} ${req}${desc}`);
        }
        console.log('');
    }
    console.log('=========================================\n');
}