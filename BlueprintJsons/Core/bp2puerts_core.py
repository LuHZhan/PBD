#!/usr/bin/env python3
"""
bp2puerts_core.py - Blueprint JSON to PuerTs 核心转换模块

这是纯粹的转换逻辑模块，不包含任何UI或CLI代码。
可以被其他模块导入使用。

使用示例:
    from bp2puerts_core import BlueprintConverter

    converter = BlueprintConverter('BP_Player.json')
    result = converter.convert()
    print(result.code)

作者: Claude
版本: 1.0.0
"""

import json
import re
from pathlib import Path
from typing import Dict, List, Any, Optional, Tuple, Set
from dataclasses import dataclass, field
from enum import Enum, auto


# ============================================
# 配置类
# ============================================

@dataclass
class ConvertConfig:
    """转换配置"""
    # 输出选项
    include_comments: bool = True           # 包含注释
    include_bytecode_hints: bool = True     # 包含字节码提示
    max_properties: int = 100               # 最大属性数量 (0=无限)
    max_functions: int = 50                 # 最大函数数量 (0=无限)

    # 代码风格
    indent: str = '    '                    # 缩进字符
    use_async_delay: bool = True            # 使用async/await处理Delay
    generate_helper_functions: bool = True  # 生成辅助函数

    # 类型推断
    infer_enum_types: bool = True           # 推断枚举类型
    use_strict_types: bool = False          # 使用严格类型 (少用any)

    # 分组
    group_properties: bool = True           # 按类别分组属性
    group_functions: bool = True            # 按类别分组函数

    # 过滤
    skip_internal_properties: bool = True   # 跳过内部属性
    skip_temp_variables: bool = True        # 跳过临时变量


# ============================================
# 类型映射表
# ============================================

class TypeMaps:
    """类型映射表集合"""

    # 属性类型映射
    PROPERTY_TYPES: Dict[str, str] = {
        'BoolProperty': 'boolean',
        'IntProperty': 'number',
        'Int64Property': 'bigint',
        'FloatProperty': 'number',
        'DoubleProperty': 'number',
        'StrProperty': 'string',
        'NameProperty': 'string',
        'TextProperty': 'string',
        'ByteProperty': 'number',
        'ObjectProperty': 'UE.Object',
        'ClassProperty': 'UE.Class',
        'SoftObjectProperty': 'UE.SoftObjectPath',
        'SoftClassProperty': 'UE.SoftClassPath',
        'StructProperty': 'any',
        'ArrayProperty': 'any[]',
        'MapProperty': 'Map<any, any>',
        'SetProperty': 'Set<any>',
        'EnumProperty': 'number',
        'DelegateProperty': 'any',
        'MulticastDelegateProperty': 'any',
        'MulticastInlineDelegateProperty': 'any',
        'InterfaceProperty': 'any',
        'WeakObjectProperty': 'UE.Object',
        'LazyObjectProperty': 'UE.Object',
    }

    # 父类映射
    PARENT_CLASSES: Dict[str, str] = {
        'Actor': 'UE.Actor',
        'Pawn': 'UE.Pawn',
        'Character': 'UE.Character',
        'PlayerController': 'UE.PlayerController',
        'AIController': 'UE.AIController',
        'GameModeBase': 'UE.GameModeBase',
        'GameMode': 'UE.GameMode',
        'GameStateBase': 'UE.GameStateBase',
        'GameState': 'UE.GameState',
        'PlayerState': 'UE.PlayerState',
        'HUD': 'UE.HUD',
        'UserWidget': 'UE.UserWidget',
        'ActorComponent': 'UE.ActorComponent',
        'SceneComponent': 'UE.SceneComponent',
        'PrimitiveComponent': 'UE.PrimitiveComponent',
        'AnimInstance': 'UE.AnimInstance',
        'BlueprintFunctionLibrary': 'UE.BlueprintFunctionLibrary',
    }

    # 组件类型
    COMPONENT_TYPES: Set[str] = {
        'SceneComponent', 'StaticMeshComponent', 'SkeletalMeshComponent',
        'CameraComponent', 'SpringArmComponent', 'BoxComponent',
        'SphereComponent', 'CapsuleComponent', 'AudioComponent',
        'WidgetComponent', 'ArrowComponent', 'SplineComponent',
        'PointLightComponent', 'SpotLightComponent', 'DirectionalLightComponent',
        'CharacterMovementComponent', 'ProjectileMovementComponent',
        'RotatingMovementComponent', 'InterpToMovementComponent',
        'ChildActorComponent', 'DecalComponent', 'TextRenderComponent',
        'BillboardComponent', 'ParticleSystemComponent', 'NiagaraComponent',
        'TimelineComponent', 'InputComponent', 'NavMeshComponent',
        # 额外添加
        'PhysicsConstraintComponent', 'PrimitiveComponent', 'ShapeComponent',
        'PostProcessComponent', 'ForceFeedbackComponent', 'PlanarReflectionComponent',
        'SceneCaptureComponent', 'SceneCaptureComponent2D', 'SceneCaptureComponentCube',
        'TextRenderComponent', 'VectorFieldComponent', 'RadialForceComponent',
        'ExponentialHeightFogComponent', 'AtmosphericFogComponent', 'SkyAtmosphereComponent',
        'RuntimeVirtualTextureComponent', 'HierarchicalInstancedStaticMeshComponent',
        'InstancedStaticMeshComponent', 'SplineMeshComponent', 'LandscapeComponent',
    }

    # 常见结构体映射
    STRUCT_TYPES: Dict[str, str] = {
        'Vector': 'UE.Vector',
        'Vector2D': 'UE.Vector2D',
        'Vector4': 'UE.Vector4',
        'Rotator': 'UE.Rotator',
        'Transform': 'UE.Transform',
        'Quat': 'UE.Quat',
        'LinearColor': 'UE.LinearColor',
        'Color': 'UE.Color',
        'Guid': 'string',
        'DateTime': 'UE.DateTime',
        'Timespan': 'UE.Timespan',
        'HitResult': 'UE.HitResult',
        'LatentActionInfo': 'UE.LatentActionInfo',
    }

    # 字节码操作映射
    BYTECODE_OPS: Dict[str, str] = {
        'EX_LocalVariable': 'var',
        'EX_InstanceVariable': 'this.',
        'EX_LocalOutVariable': 'out',
        'EX_Self': 'this',
        'EX_IntConst': 'int',
        'EX_FloatConst': 'float',
        'EX_DoubleConst': 'double',
        'EX_StringConst': 'string',
        'EX_NameConst': 'name',
        'EX_True': 'true',
        'EX_False': 'false',
        'EX_Nothing': 'void',
        'EX_CallMath': 'Math.',
        'EX_FinalFunction': 'Call',
        'EX_LocalFinalFunction': 'LocalCall',
        'EX_VirtualFunction': 'Virtual',
        'EX_Let': '=',
        'EX_LetBool': '= (bool)',
        'EX_LetObj': '= (obj)',
        'EX_Return': 'return',
        'EX_EndOfScript': 'end',
        'EX_Jump': 'goto',
        'EX_JumpIfNot': 'if(!)',
        'EX_ComputedJump': 'switch',
        'EX_PushExecutionFlow': '// LatentStart',
        'EX_PopExecutionFlow': '// LatentResume',
        'EX_Context': 'context->',
        'EX_StructConst': 'struct',
        'EX_SetArray': 'array=',
    }


# ============================================
# 数据类
# ============================================

class PropertyCategory(Enum):
    """属性分类"""
    COMPONENT = auto()
    CONFIG = auto()
    STATE = auto()
    REFERENCE = auto()
    OTHER = auto()


class FunctionCategory(Enum):
    """函数分类"""
    EVENT = auto()
    INPUT = auto()
    CALLABLE = auto()
    PURE = auto()
    INTERNAL = auto()


@dataclass
class ParsedProperty:
    """解析后的属性"""
    name: str
    original_name: str
    type: str
    serialized_type: str
    flags: str = ''
    category: PropertyCategory = PropertyCategory.OTHER
    component_class: str = ''
    default_value: Any = None
    description: str = ''
    parent_component: str = ''  # 父组件名
    children: List[str] = field(default_factory=list)  # 子组件名列表
    is_native_parent: bool = False  # 父组件是否是原生C++组件


@dataclass
class ComponentNode:
    """组件节点 - 用于构建层级树"""
    name: str
    component_type: str
    parent: Optional[str] = None
    children: List[str] = field(default_factory=list)
    is_native_parent: bool = False
    template_index: int = 0


@dataclass
class FunctionParam:
    """函数参数"""
    name: str
    type: str
    is_out: bool = False
    is_ref: bool = False
    default_value: Any = None


@dataclass
class ParsedFunction:
    """解析后的函数"""
    name: str
    original_name: str
    params: List[FunctionParam] = field(default_factory=list)
    return_type: str = 'void'
    category: FunctionCategory = FunctionCategory.CALLABLE
    flags: str = ''
    bytecode_hints: List[str] = field(default_factory=list)
    has_latent: bool = False
    description: str = ''


@dataclass
class ParsedBlueprint:
    """解析后的蓝图"""
    class_name: str
    original_name: str
    parent_class: str
    folder_name: str
    properties: List[ParsedProperty] = field(default_factory=list)
    functions: List[ParsedFunction] = field(default_factory=list)
    enums: List[str] = field(default_factory=list)
    interfaces: List[str] = field(default_factory=list)
    component_hierarchy: Dict[str, ComponentNode] = field(default_factory=dict)  # 组件层级


@dataclass
class ConvertResult:
    """转换结果"""
    success: bool
    code: str
    blueprint: Optional[ParsedBlueprint] = None
    warnings: List[str] = field(default_factory=list)
    errors: List[str] = field(default_factory=list)
    stats: Dict[str, int] = field(default_factory=dict)


# ============================================
# 解析器
# ============================================

class BlueprintParser:
    """蓝图JSON解析器"""

    def __init__(self, config: ConvertConfig = None):
        self.config = config or ConvertConfig()
        self.data: Dict = {}
        self.imports: Dict[int, dict] = {}
        self.name_map: List[str] = []
        self.warnings: List[str] = []

    def load(self, json_path: str) -> None:
        """加载JSON文件"""
        with open(json_path, 'r', encoding='utf-8') as f:
            self.data = json.load(f)

        self.name_map = self.data.get('NameMap', [])
        self._build_import_map()

    def load_from_string(self, json_string: str) -> None:
        """从字符串加载JSON"""
        self.data = json.loads(json_string)
        self.name_map = self.data.get('NameMap', [])
        self._build_import_map()

    def _build_import_map(self) -> None:
        """构建Import索引映射"""
        self.imports.clear()
        for i, imp in enumerate(self.data.get('Imports', [])):
            self.imports[-(i + 1)] = imp

    def _sanitize_name(self, name: str) -> str:
        """将名称转换为有效的TypeScript标识符"""
        if not name:
            return 'unnamed'

        # 移除特殊字符
        result = re.sub(r'[\[\](){}]', '', name)
        result = re.sub(r'[^a-zA-Z0-9_]', '_', result)
        result = re.sub(r'^(\d)', r'_\1', result)
        result = re.sub(r'_+', '_', result)
        result = result.strip('_')

        return result or 'unnamed'

    def _resolve_import(self, index: int) -> Optional[dict]:
        """解析Import引用"""
        return self.imports.get(index)

    def _get_property_type(self, prop: dict) -> str:
        """获取属性类型"""
        serialized_type = prop.get('SerializedType', 'Unknown')
        base_type = TypeMaps.PROPERTY_TYPES.get(serialized_type, 'any')

        # ObjectProperty - 尝试获取具体类型
        if serialized_type == 'ObjectProperty':
            prop_class = prop.get('PropertyClass')
            if prop_class:
                imp = self._resolve_import(prop_class)
                if imp:
                    class_name = imp.get('ClassName', '')
                    return f"UE.{class_name}" if class_name else base_type

        # StructProperty - 尝试获取结构类型
        if serialized_type == 'StructProperty':
            struct = prop.get('Struct')
            if struct:
                imp = self._resolve_import(struct)
                if imp:
                    struct_name = imp.get('ObjectName', '')
                    return TypeMaps.STRUCT_TYPES.get(struct_name, f"UE.{struct_name}")

        # ByteProperty with Enum
        if prop.get('Enum'):
            imp = self._resolve_import(prop['Enum'])
            if imp:
                return imp.get('ObjectName', 'number')

        return base_type

    def _classify_property(self, prop: dict) -> Tuple[PropertyCategory, str]:
        """分类属性"""
        name = prop.get('Name', '')
        serialized_type = prop.get('SerializedType', '')

        # 检查是否是组件
        if serialized_type == 'ObjectProperty':
            prop_class = prop.get('PropertyClass')
            if prop_class:
                imp = self._resolve_import(prop_class)
                if imp:
                    # 使用 ObjectName 获取实际类名
                    class_name = imp.get('ObjectName', '')

                    # 检查是否在已知组件类型列表中
                    if class_name in TypeMaps.COMPONENT_TYPES:
                        return PropertyCategory.COMPONENT, class_name

                    # 检查是否以 Component 结尾 (动态识别)
                    if class_name.endswith('Component'):
                        return PropertyCategory.COMPONENT, class_name

                    # 检查是否是蓝图组件类 (以 _C 结尾且包含 Comp 或 Component)
                    if class_name.endswith('_C') and ('Comp' in class_name or 'Component' in class_name):
                        return PropertyCategory.COMPONENT, class_name

        # 检查是否是配置属性
        if name.startswith('[Config') or name.startswith('Config_'):
            return PropertyCategory.CONFIG, ''

        # 检查是否是引用
        if serialized_type == 'ObjectProperty':
            return PropertyCategory.REFERENCE, ''

        return PropertyCategory.OTHER, ''

    def _classify_function(self, func_flags: str, name: str) -> FunctionCategory:
        """分类函数"""
        if 'FUNC_Event' in func_flags or 'FUNC_BlueprintEvent' in func_flags:
            return FunctionCategory.EVENT

        if 'FUNC_BlueprintPure' in func_flags:
            return FunctionCategory.PURE

        # 检查是否是输入相关
        input_keywords = ['Move', 'Look', 'Turn', 'Input', 'Axis']
        if any(kw in name for kw in input_keywords):
            return FunctionCategory.INPUT

        if 'FUNC_BlueprintCallable' in func_flags:
            return FunctionCategory.CALLABLE

        return FunctionCategory.INTERNAL

    def _parse_bytecode(self, bytecode: List[dict]) -> Tuple[List[str], bool]:
        """解析字节码，返回提示和是否包含Latent"""
        if not bytecode:
            return [], False

        hints = []
        has_latent = False

        for inst in bytecode[:30]:
            op_type = inst.get('$type', '').split('.')[-1].replace(', UAssetAPI', '')

            if op_type == 'EX_PushExecutionFlow':
                has_latent = True
                hints.append("// Latent action point")

            elif op_type in ('EX_CallMath', 'EX_FinalFunction', 'EX_LocalFinalFunction'):
                stack_node = inst.get('StackNode')
                if stack_node and stack_node < 0:
                    imp = self._resolve_import(stack_node)
                    if imp:
                        func_name = imp.get('ObjectName', 'Unknown')
                        class_name = imp.get('ClassName', '')
                        if class_name:
                            hints.append(f"// {class_name}.{func_name}()")
                        else:
                            hints.append(f"// {func_name}()")

                        # 检查是否是Delay
                        if func_name == 'Delay':
                            has_latent = True

            elif op_type == 'EX_Let':
                hints.append("// Assignment")

            elif op_type == 'EX_Return':
                hints.append("// return")
                break

            elif op_type == 'EX_EndOfScript':
                break

        return hints, has_latent

    def parse(self) -> ParsedBlueprint:
        """解析蓝图"""
        class_name = 'UnknownClass'
        original_name = ''
        parent_class = 'UE.Actor'
        folder_name = self.data.get('FolderName', '')
        properties: List[ParsedProperty] = []
        functions: List[ParsedFunction] = []
        enums: List[str] = []
        interfaces: List[str] = []

        # 解析Exports
        for exp in self.data.get('Exports', []):
            exp_type = exp.get('$type', '')

            # ClassExport - 主类定义
            if 'ClassExport' in exp_type:
                original_name = exp.get('ObjectName', 'UnknownClass')
                class_name = self._sanitize_name(original_name.replace('_C', ''))

                # 父类
                super_index = exp.get('SuperIndex')
                if super_index and super_index < 0:
                    imp = self._resolve_import(super_index)
                    if imp:
                        parent_name = imp.get('ObjectName', 'Actor')
                        parent_class = TypeMaps.PARENT_CLASSES.get(
                            parent_name, f"UE.{parent_name}"
                        )

                # 接口
                for iface in exp.get('Interfaces', []):
                    if isinstance(iface, dict):
                        interfaces.append(iface.get('Class', ''))

                # 属性
                for prop in exp.get('LoadedProperties', []):
                    prop_name = prop.get('Name', 'unnamed')

                    # 跳过内部属性
                    if self.config.skip_internal_properties:
                        if prop_name.startswith('UberGraph'):
                            continue

                    category, comp_class = self._classify_property(prop)

                    properties.append(ParsedProperty(
                        name=self._sanitize_name(prop_name),
                        original_name=prop_name,
                        type=f"UE.{comp_class}" if comp_class else self._get_property_type(prop),
                        serialized_type=prop.get('SerializedType', 'Unknown'),
                        flags=prop.get('PropertyFlags', ''),
                        category=category,
                        component_class=comp_class,
                    ))

            # FunctionExport - 函数定义
            elif 'FunctionExport' in exp_type:
                func_name = exp.get('ObjectName', 'unnamed')

                # 跳过UberGraph函数
                if func_name.startswith('ExecuteUbergraph'):
                    continue

                func_flags = exp.get('FunctionFlags', '')
                category = self._classify_function(func_flags, func_name)

                params: List[FunctionParam] = []
                return_type = 'void'

                for prop in exp.get('LoadedProperties', []):
                    prop_name = prop.get('Name', '')
                    prop_flags = prop.get('PropertyFlags', '')

                    # 跳过临时变量
                    if self.config.skip_temp_variables and prop_name.startswith('CallFunc_'):
                        continue

                    if 'CPF_ReturnParm' in prop_flags:
                        return_type = self._get_property_type(prop)
                    elif 'CPF_Parm' in prop_flags:
                        params.append(FunctionParam(
                            name=self._sanitize_name(prop_name),
                            type=self._get_property_type(prop),
                            is_out='CPF_OutParm' in prop_flags,
                            is_ref='CPF_ReferenceParm' in prop_flags,
                        ))

                bytecode_hints, has_latent = self._parse_bytecode(
                    exp.get('ScriptBytecode', [])
                )

                functions.append(ParsedFunction(
                    name=self._sanitize_name(func_name),
                    original_name=func_name,
                    params=params,
                    return_type=return_type,
                    category=category,
                    flags=func_flags,
                    bytecode_hints=bytecode_hints if self.config.include_bytecode_hints else [],
                    has_latent=has_latent,
                ))

        # 从NameMap提取枚举
        if self.config.infer_enum_types:
            for name in self.name_map:
                if name.startswith('E_') or name.startswith('EAnimBS'):
                    enums.append(name)

        # 解析组件层级结构
        component_hierarchy = self._parse_component_hierarchy()

        # 将层级信息合并到属性中
        self._merge_hierarchy_to_properties(properties, component_hierarchy)

        return ParsedBlueprint(
            class_name=class_name,
            original_name=original_name,
            parent_class=parent_class,
            folder_name=folder_name,
            properties=properties,
            functions=functions,
            enums=list(set(enums)),
            interfaces=interfaces,
            component_hierarchy=component_hierarchy,
        )

    def _get_scs_node_data(self, exp: dict) -> dict:
        """从SCS_Node Export中提取数据"""
        result = {}
        for key in ['Data', 'ExtraData']:
            if key in exp and isinstance(exp[key], list):
                for item in exp[key]:
                    if isinstance(item, dict) and 'Name' in item:
                        name = item['Name']
                        val = item.get('Value', item.get('$value'))
                        result[name] = val
        return result

    def _parse_component_hierarchy(self) -> Dict[str, ComponentNode]:
        """解析组件层级结构（从SCS_Node）"""
        hierarchy: Dict[str, ComponentNode] = {}
        exports = self.data.get('Exports', [])

        # 第一遍：收集所有SCS_Node
        scs_nodes = {}  # export_index -> node_data
        for i, exp in enumerate(exports):
            class_index = exp.get('ClassIndex', 0)
            if class_index < 0:
                imp = self._resolve_import(class_index)
                if imp and imp.get('ObjectName') == 'SCS_Node':
                    node_data = self._get_scs_node_data(exp)
                    internal_name = node_data.get('InternalVariableName', '')
                    if internal_name:
                        # 获取组件类型
                        comp_class_idx = node_data.get('ComponentClass', 0)
                        comp_type = ''
                        if comp_class_idx and comp_class_idx < 0:
                            imp = self._resolve_import(comp_class_idx)
                            if imp:
                                comp_type = imp.get('ObjectName', '')

                        scs_nodes[i + 1] = {
                            'name': internal_name,
                            'parent': node_data.get('ParentComponentOrVariableName'),
                            'is_native_parent': node_data.get('bIsParentComponentNative', False),
                            'children_indices': [],
                            'component_type': comp_type,
                            'template_index': node_data.get('ComponentTemplate', 0),
                        }

                        # 提取子节点索引
                        child_nodes = node_data.get('ChildNodes', [])
                        if isinstance(child_nodes, list):
                            for child in child_nodes:
                                if isinstance(child, dict):
                                    child_idx = child.get('Value', 0)
                                    if child_idx:
                                        scs_nodes[i + 1]['children_indices'].append(child_idx)

        # 第二遍：建立层级关系，包括设置子节点的父组件
        for exp_idx, node in scs_nodes.items():
            # 设置子节点的父组件
            for child_idx in node['children_indices']:
                if child_idx in scs_nodes:
                    child_node = scs_nodes[child_idx]
                    # 如果子节点没有设置父组件，设置为当前节点
                    if not child_node['parent']:
                        child_node['parent'] = node['name']
                        child_node['is_native_parent'] = False

        # 第三遍：创建 ComponentNode 对象
        for exp_idx, node in scs_nodes.items():
            comp_node = ComponentNode(
                name=node['name'],
                component_type=node['component_type'],
                parent=node['parent'] if node['parent'] else None,
                is_native_parent=node['is_native_parent'],
                template_index=node['template_index'],
            )

            # 解析子节点名称
            for child_idx in node['children_indices']:
                if child_idx in scs_nodes:
                    comp_node.children.append(scs_nodes[child_idx]['name'])

            hierarchy[node['name']] = comp_node

        return hierarchy

    def _merge_hierarchy_to_properties(self, properties: List[ParsedProperty],
                                        hierarchy: Dict[str, ComponentNode]) -> None:
        """将层级信息合并到属性中"""
        # 建立名称映射（处理名称变体）
        name_map = {}
        for prop in properties:
            # 原始名可能包含空格等，标准化处理
            clean_name = prop.original_name.replace('_GEN_VARIABLE', '')
            name_map[clean_name] = prop
            name_map[prop.name] = prop

        for comp_name, node in hierarchy.items():
            # 查找匹配的属性
            prop = name_map.get(comp_name)
            if not prop:
                # 尝试标准化后查找
                sanitized = self._sanitize_name(comp_name)
                prop = name_map.get(sanitized)

            if prop:
                prop.parent_component = node.parent or ''
                prop.children = node.children.copy()
                prop.is_native_parent = node.is_native_parent


# ============================================
# 代码生成器
# ============================================

class CodeGenerator:
    """TypeScript代码生成器"""

    def __init__(self, config: ConvertConfig = None):
        self.config = config or ConvertConfig()
        self.indent = self.config.indent

    def generate(self, blueprint: ParsedBlueprint) -> str:
        """生成TypeScript代码"""
        lines: List[str] = []

        # 文件头
        lines.extend(self._generate_header(blueprint))

        # 导入
        lines.extend(self._generate_imports())
        lines.append('')

        # 类型扩展声明 (解决 GetMesh 等方法的类型问题)
        type_ext = self._generate_type_extensions(blueprint)
        if type_ext:
            lines.extend(type_ext)
            lines.append('')

        # 辅助函数
        if self.config.generate_helper_functions:
            lines.extend(self._generate_helpers())
            lines.append('')

        # 类定义
        lines.extend(self._generate_class(blueprint))

        # 导出
        lines.append('')
        lines.append(f"export default {blueprint.class_name};")

        return '\n'.join(lines)

    def _generate_header(self, bp: ParsedBlueprint) -> List[str]:
        """生成文件头注释"""
        if not self.config.include_comments:
            return []

        return [
            '/**',
            f' * {bp.class_name}',
            f' * 从蓝图转换: {bp.folder_name}',
            f' * 原始类名: {bp.original_name}',
            ' *',
            ' * 由 bp2puerts 自动生成',
            ' */',
            '',
        ]

    def _generate_imports(self) -> List[str]:
        """生成导入语句"""
        return [
            "/// <reference path=\"./ue_extensions.d.ts\" />",
            "import * as UE from 'ue'",
            "import {$ref, $unref} from 'puerts'",
        ]

    def _generate_type_extensions(self, bp: ParsedBlueprint) -> List[str]:
        """生成类型扩展提示（不再内联，改为引用外部文件）"""
        # 类型扩展现在通过 ue_extensions.d.ts 文件提供
        # 这里只生成注释提示
        return []

    def _generate_helpers(self) -> List[str]:
        """生成辅助函数"""
        return [
            '/**',
            ' * 延迟函数 - 用于处理Delay节点',
            ' */',
            'function delay(ms: number): Promise<void> {',
            f'{self.indent}return new Promise(resolve => setTimeout(resolve, ms));',
            '}',
        ]

    def _generate_class(self, bp: ParsedBlueprint) -> List[str]:
        """生成类定义"""
        lines: List[str] = []

        # 类声明
        lines.append(f"class {bp.class_name} extends {bp.parent_class} {{")

        # 属性
        if self.config.group_properties:
            lines.extend(self._generate_grouped_properties(bp.properties))
        else:
            lines.extend(self._generate_flat_properties(bp.properties))

        # Constructor
        lines.extend(self._generate_constructor())

        # 函数
        if self.config.group_functions:
            lines.extend(self._generate_grouped_functions(bp.functions))
        else:
            lines.extend(self._generate_flat_functions(bp.functions))

        lines.append('}')

        return lines

    def _generate_grouped_properties(self, properties: List[ParsedProperty]) -> List[str]:
        """按分类生成属性"""
        lines: List[str] = []

        # 按分类分组
        groups: Dict[PropertyCategory, List[ParsedProperty]] = {
            cat: [] for cat in PropertyCategory
        }
        for prop in properties:
            groups[prop.category].append(prop)

        # 组件 - 按层级组织
        components = groups[PropertyCategory.COMPONENT]
        if components:
            lines.append('')
            lines.append(f'{self.indent}// ========== 组件 ==========')
            lines.append(f'{self.indent}// 组件层级结构 (在蓝图中按此结构添加组件):')

            # 构建层级树
            root_comps = []
            native_children = []
            other_comps = []

            for prop in components:
                if not prop.parent_component:
                    root_comps.append(prop)
                elif prop.is_native_parent:
                    native_children.append(prop)
                else:
                    other_comps.append(prop)

            # 输出原生组件的子组件 (挂载到 Capsule/Mesh 等)
            if native_children:
                lines.append(f'{self.indent}// └─ [挂载到原生组件]')
                for prop in native_children:
                    parent_info = f'  // 父: {prop.parent_component}'
                    children_info = f' 子: [{", ".join(prop.children)}]' if prop.children else ''
                    lines.append(f'{self.indent}{prop.name}: {prop.type};{parent_info}{children_info}')

            # 输出根组件
            if root_comps:
                lines.append(f'{self.indent}// └─ [独立组件]')
                for prop in root_comps:
                    children_info = f'  // 子: [{", ".join(prop.children)}]' if prop.children else ''
                    lines.append(f'{self.indent}{prop.name}: {prop.type};{children_info}')

            # 输出其他组件 (有非原生父组件)
            if other_comps:
                lines.append(f'{self.indent}// └─ [子组件]')
                for prop in other_comps:
                    parent_info = f'  // 父: {prop.parent_component}'
                    lines.append(f'{self.indent}{prop.name}: {prop.type};{parent_info}')

        # 配置属性
        if groups[PropertyCategory.CONFIG]:
            lines.append('')
            lines.append(f'{self.indent}// ========== 配置 ==========')
            count = 0
            for prop in groups[PropertyCategory.CONFIG]:
                if self.config.max_properties and count >= self.config.max_properties:
                    lines.append(f'{self.indent}// ... 还有更多配置')
                    break
                lines.append(f'{self.indent}{prop.name}: {prop.type};')
                count += 1

        # 状态属性
        other_props = groups[PropertyCategory.STATE] + groups[PropertyCategory.REFERENCE] + groups[PropertyCategory.OTHER]
        if other_props:
            lines.append('')
            lines.append(f'{self.indent}// ========== 属性 ==========')
            count = 0
            for prop in other_props:
                if self.config.max_properties and count >= self.config.max_properties:
                    lines.append(f'{self.indent}// ... 还有 {len(other_props) - count} 个属性')
                    break
                lines.append(f'{self.indent}{prop.name}: {prop.type};')
                count += 1

        return lines

    def _generate_flat_properties(self, properties: List[ParsedProperty]) -> List[str]:
        """平铺生成属性"""
        lines: List[str] = ['']
        count = 0
        for prop in properties:
            if self.config.max_properties and count >= self.config.max_properties:
                lines.append(f'{self.indent}// ... 还有 {len(properties) - count} 个属性')
                break
            lines.append(f'{self.indent}{prop.name}: {prop.type};')
            count += 1
        return lines

    def _generate_constructor(self) -> List[str]:
        """生成构造函数"""
        return [
            '',
            f'{self.indent}// ========== 构造函数 ==========',
            '',
            f'{self.indent}Constructor() {{',
            f'{self.indent}{self.indent}// 初始化',
            f'{self.indent}}}',
        ]

    def _generate_grouped_functions(self, functions: List[ParsedFunction]) -> List[str]:
        """按分类生成函数"""
        lines: List[str] = []

        # 按分类分组
        groups: Dict[FunctionCategory, List[ParsedFunction]] = {
            cat: [] for cat in FunctionCategory
        }
        for func in functions:
            groups[func.category].append(func)

        # 事件
        if groups[FunctionCategory.EVENT]:
            lines.append('')
            lines.append(f'{self.indent}// ========== 事件 ==========')
            for func in groups[FunctionCategory.EVENT]:
                lines.extend(self._generate_function(func))

        # 输入
        if groups[FunctionCategory.INPUT]:
            lines.append('')
            lines.append(f'{self.indent}// ========== 输入 ==========')
            for func in groups[FunctionCategory.INPUT]:
                lines.extend(self._generate_function(func))

        # 可调用函数
        callable_funcs = groups[FunctionCategory.CALLABLE] + groups[FunctionCategory.PURE]
        if callable_funcs:
            lines.append('')
            lines.append(f'{self.indent}// ========== 函数 ==========')
            count = 0
            for func in callable_funcs:
                if self.config.max_functions and count >= self.config.max_functions:
                    lines.append(f'{self.indent}// ... 还有 {len(callable_funcs) - count} 个函数')
                    break
                lines.extend(self._generate_function(func))
                count += 1

        return lines

    def _generate_flat_functions(self, functions: List[ParsedFunction]) -> List[str]:
        """平铺生成函数"""
        lines: List[str] = []
        count = 0
        for func in functions:
            if self.config.max_functions and count >= self.config.max_functions:
                lines.append(f'{self.indent}// ... 还有 {len(functions) - count} 个函数')
                break
            lines.extend(self._generate_function(func))
            count += 1
        return lines

    def _generate_function(self, func: ParsedFunction) -> List[str]:
        """生成单个函数"""
        lines: List[str] = []

        # 参数字符串
        params_parts = []
        for p in func.params:
            prefix = '/* out */ ' if p.is_out else ''
            params_parts.append(f"{prefix}{p.name}: {p.type}")
        params_str = ', '.join(params_parts)

        # 是否异步
        is_async = func.has_latent and self.config.use_async_delay
        return_type = f"Promise<{func.return_type}>" if is_async else func.return_type
        async_prefix = 'async ' if is_async else ''

        # 注释
        if self.config.include_comments:
            lines.append('')
            lines.append(f'{self.indent}/**')
            lines.append(f'{self.indent} * {func.original_name}')
            if func.category == FunctionCategory.EVENT:
                lines.append(f'{self.indent} * @event')
            lines.append(f'{self.indent} */')

        # 装饰器
        if is_async:
            lines.append(f'{self.indent}//@no-blueprint')

        # 函数签名
        lines.append(f'{self.indent}{async_prefix}{func.name}({params_str}): {return_type} {{')

        # 函数体
        if func.bytecode_hints:
            for hint in func.bytecode_hints[:5]:
                lines.append(f'{self.indent}{self.indent}{hint}')
            if len(func.bytecode_hints) > 5:
                lines.append(f'{self.indent}{self.indent}// ...')
        else:
            lines.append(f'{self.indent}{self.indent}// TODO: 实现')

        lines.append(f'{self.indent}}}')

        return lines


# ============================================
# 主转换器
# ============================================

class BlueprintConverter:
    """蓝图转换器 - 主入口类"""

    def __init__(self, json_path: str = None, config: ConvertConfig = None):
        self.config = config or ConvertConfig()
        self.parser = BlueprintParser(self.config)
        self.generator = CodeGenerator(self.config)
        self.json_path = json_path

        if json_path:
            self.load(json_path)

    def load(self, json_path: str) -> None:
        """加载蓝图JSON文件"""
        self.json_path = json_path
        self.parser.load(json_path)

    def load_from_string(self, json_string: str) -> None:
        """从字符串加载"""
        self.parser.load_from_string(json_string)

    def parse(self) -> ParsedBlueprint:
        """解析蓝图"""
        return self.parser.parse()

    def convert(self) -> ConvertResult:
        """执行转换"""
        try:
            blueprint = self.parser.parse()
            code = self.generator.generate(blueprint)

            # 统计信息
            stats = {
                'properties': len(blueprint.properties),
                'components': sum(1 for p in blueprint.properties if p.category == PropertyCategory.COMPONENT),
                'functions': len(blueprint.functions),
                'events': sum(1 for f in blueprint.functions if f.category == FunctionCategory.EVENT),
                'lines': code.count('\n') + 1,
            }

            return ConvertResult(
                success=True,
                code=code,
                blueprint=blueprint,
                warnings=self.parser.warnings,
                stats=stats,
            )

        except Exception as e:
            return ConvertResult(
                success=False,
                code='',
                errors=[str(e)],
            )

    def save(self, output_path: str) -> bool:
        """转换并保存"""
        result = self.convert()
        if result.success:
            Path(output_path).write_text(result.code, encoding='utf-8')
            return True
        return False


# ============================================
# 便捷函数
# ============================================

def convert_file(input_path: str, output_path: str = None, config: ConvertConfig = None) -> ConvertResult:
    """转换单个文件"""
    converter = BlueprintConverter(input_path, config)
    result = converter.convert()

    if result.success and output_path:
        Path(output_path).write_text(result.code, encoding='utf-8')

    return result


def convert_string(json_string: str, config: ConvertConfig = None) -> ConvertResult:
    """转换JSON字符串"""
    converter = BlueprintConverter(config=config)
    converter.load_from_string(json_string)
    return converter.convert()


# ============================================
# 版本信息
# ============================================

__version__ = '1.0.0'
__author__ = 'Claude'
