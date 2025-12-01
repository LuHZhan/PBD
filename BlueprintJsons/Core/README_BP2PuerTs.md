# Blueprint JSON to PuerTs 转换工具

## 概述

这套工具将UAssetGUI导出的蓝图JSON文件转换为PuerTs TypeScript代码。

## 架构设计

```
┌─────────────────────────────────────────────────────────────┐
│                      bp2puerts.py                           │
│                      (统一入口)                              │
└─────────────────┬───────────────────┬───────────────────────┘
                  │                   │
         ┌────────▼────────┐ ┌────────▼────────┐
         │  bp2puerts_cli  │ │  bp2puerts_gui  │
         │   (命令行工具)   │ │   (图形界面)     │
         └────────┬────────┘ └────────┬────────┘
                  │                   │
                  └─────────┬─────────┘
                            │
                  ┌─────────▼─────────┐
                  │  bp2puerts_core   │
                  │   (核心转换模块)   │
                  │                   │
                  │ • BlueprintParser │
                  │ • CodeGenerator   │
                  │ • ConvertConfig   │
                  └───────────────────┘
```

## 文件说明

| 文件 | 说明 |
|------|------|
| `bp2puerts.py` | 统一入口 - 自动选择CLI或GUI |
| `bp2puerts_core.py` | 核心模块 - 解析和代码生成 |
| `bp2puerts_cli.py` | 命令行工具 - 参数解析、批量处理 |
| `bp2puerts_gui.py` | 图形界面 - tkinter实现 |
| `BlueprintConverter.ts` | TypeScript版本 (备用) |
| `CharBP_Base.ts` | 转换示例 |

## 快速开始

### 启动GUI (无参数)
```bash
python bp2puerts.py
```

### 命令行转换
```bash
# 基本使用
python bp2puerts.py BP_Player.json

# 指定输出
python bp2puerts.py BP_Player.json -o TS_Player.ts

# 批量转换
python bp2puerts.py *.json -d output_dir/

# 详细输出
python bp2puerts.py BP_Player.json -v

# 只查看信息
python bp2puerts.py BP_Player.json --info

# 预览不保存
python bp2puerts.py BP_Player.json --preview
```

### 强制指定模式
```bash
# 强制GUI
python bp2puerts.py --gui

# 强制CLI
python bp2puerts.py --cli BP_Player.json
```

## CLI 完整选项

```
用法: bp2puerts.py [-h] [-o OUTPUT] [-d OUTPUT_DIR] [-c CONFIG]
                   [--init-config PATH] [--no-comments] [--no-bytecode]
                   [--no-helpers] [--flat] [--max-props N] [--max-funcs N]
                   [-v] [--info] [--preview] [--no-color] [-V]
                   [input ...]

输出选项:
  -o, --output        输出文件路径
  -d, --output-dir    输出目录 (批量转换)

配置选项:
  -c, --config        配置文件路径
  --init-config PATH  生成默认配置文件

转换选项:
  --no-comments       不生成注释
  --no-bytecode       不包含字节码提示
  --no-helpers        不生成辅助函数
  --flat              不分组属性和函数
  --max-props N       最大属性数量
  --max-funcs N       最大函数数量

显示选项:
  -v, --verbose       详细输出
  --info              只显示蓝图信息，不转换
  --preview           预览输出，不保存
  --no-color          禁用彩色输出

其他:
  -V, --version       显示版本
```

## 配置文件格式

使用 `--init-config config.json` 生成默认配置:

```json
{
  "include_comments": true,
  "include_bytecode_hints": true,
  "max_properties": 100,
  "max_functions": 50,
  "indent": "    ",
  "use_async_delay": true,
  "generate_helper_functions": true,
  "infer_enum_types": true,
  "use_strict_types": false,
  "group_properties": true,
  "group_functions": true,
  "skip_internal_properties": true,
  "skip_temp_variables": true
}
```

## 蓝图JSON获取方法

1. **下载UAssetGUI**: https://github.com/atenfyr/UAssetGUI
2. **打开.uasset文件**: 找到你的蓝图 `.uasset` 文件
3. **导出JSON**: File -> Save As -> 选择 JSON 格式

## 核心API使用

### 基本使用
```python
from bp2puerts_core import BlueprintConverter

# 创建转换器
converter = BlueprintConverter('BP_Player.json')

# 执行转换
result = converter.convert()

if result.success:
    print(result.code)
    print(f"属性: {result.stats['properties']}")
    print(f"函数: {result.stats['functions']}")
else:
    print("错误:", result.errors)
```

### 自定义配置
```python
from bp2puerts_core import BlueprintConverter, ConvertConfig

# 创建配置
config = ConvertConfig()
config.include_comments = False
config.max_properties = 50
config.group_properties = True

# 使用配置转换
converter = BlueprintConverter('BP_Player.json', config)
result = converter.convert()
```

### 便捷函数
```python
from bp2puerts_core import convert_file, convert_string

# 转换文件
result = convert_file('BP_Player.json', 'TS_Player.ts')

# 转换字符串
json_str = open('BP_Player.json').read()
result = convert_string(json_str)
```

### 访问解析结果
```python
converter = BlueprintConverter('BP_Player.json')
blueprint = converter.parse()

print(f"类名: {blueprint.class_name}")
print(f"父类: {blueprint.parent_class}")

for prop in blueprint.properties:
    print(f"属性: {prop.name} : {prop.type}")

for func in blueprint.functions:
    print(f"函数: {func.name}({', '.join(p.name for p in func.params)})")
```

## 转换结果结构

```typescript
import * as UE from 'ue'
import {$ref, $unref} from 'puerts'

function delay(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms));
}

class YourBlueprint extends UE.ParentClass {
    // ========== 组件 ==========
    Camera: UE.CameraComponent;
    
    // ========== 属性 ==========
    Health: number;
    
    // ========== 构造函数 ==========
    Constructor() {
        // 初始化
    }
    
    // ========== 事件 ==========
    ReceiveBeginPlay(): void {
        // BeginPlay逻辑
    }
    
    // ========== 函数 ==========
    CustomFunction(param: number): void {
        // 函数逻辑
    }
}

export default YourBlueprint;
```

## PuerTs绑定步骤

转换后的TypeScript文件需要与UE蓝图绑定：

1. **创建蓝图**: 在UE编辑器中创建与TS类同名的蓝图
2. **设置父类**: 选择正确的父类 (Actor, Character等)
3. **关联TypeScript**: 在蓝图的Class Settings中关联TS文件
4. **添加组件**: 按TS中声明的名称添加组件

## 支持的转换内容

### 属性类型

| 蓝图类型 | TypeScript类型 |
|----------|----------------|
| BoolProperty | `boolean` |
| IntProperty | `number` |
| FloatProperty | `number` |
| DoubleProperty | `number` |
| StrProperty | `string` |
| NameProperty | `string` |
| ObjectProperty | `UE.ClassName` |
| StructProperty | `UE.StructName` |
| ArrayProperty | `type[]` |
| MapProperty | `Map<K, V>` |

### 函数类型

| 蓝图函数类型 | 转换结果 |
|-------------|---------|
| BlueprintEvent | 普通方法 |
| BlueprintCallable | 普通方法 |
| BlueprintPure | 普通方法 (无副作用) |
| Latent Function | async方法 + await |

### 字节码解析

工具会解析蓝图字节码并生成注释形式的伪代码：

```typescript
MyFunction(): void {
    // KismetSystemLibrary.Delay()
    // Assignment
    // return
}
```

## 常见问题

### Q: 为什么有些属性类型是 `any`？

A: 无法从JSON中确定具体类型时会使用 `any`。你可以手动修改为正确的类型。

### Q: Delay节点如何处理？

A: 使用 `async/await` + 自定义 `delay()` 函数：

```typescript
async MyFunction(): Promise<void> {
    await delay(1000);  // 1秒延迟
    // 后续逻辑
}
```

### Q: 组件如何绑定？

A: 在TS中声明同名属性，然后在UE蓝图中添加同名组件：

```typescript
// TypeScript
Camera: UE.CameraComponent;

// UE蓝图中添加名为 "Camera" 的CameraComponent
```

### Q: 事件绑定如何处理？

A: 组件事件（如OnBeginOverlap）需要在蓝图中手动绑定，或使用动态绑定：

```typescript
ReceiveBeginPlay(): void {
    // 动态绑定示例
    this.MyCollisionBox.OnComponentBeginOverlap.Add(
        (OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult) => {
            this.OnMyCollisionBeginOverlap(OtherActor);
        }
    );
}
```

## 限制

1. **复杂逻辑**: 字节码无法完全还原为可执行代码，需要手动实现
2. **宏和自定义节点**: 不支持蓝图宏和自定义K2节点
3. **Timeline**: 需要手动转换为代码实现
4. **动态绑定**: 委托绑定需要额外处理

## 最佳实践

1. **分步转换**: 先转换简单蓝图，逐步处理复杂蓝图
2. **保留原蓝图**: 转换后保留原蓝图作为参考
3. **逐个验证**: 每转换一个函数就测试一次
4. **类型标注**: 手动补充 `any` 类型为具体类型
5. **注释保留**: 保留字节码注释作为实现参考

## 更新日志

- v1.0.0: 初始版本，支持基本的类/属性/函数转换
