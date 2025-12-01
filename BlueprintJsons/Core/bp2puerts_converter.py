#!/usr/bin/env python3
"""
Blueprint JSON to PuerTS TypeScript 转换工具

功能:
- 解析UAssetGUI导出的蓝图JSON
- 提取组件层级结构
- 提取变量定义
- 反编译字节码生成函数实现
- 生成PuerTS TypeScript代码

使用方法:
    python bp2puerts_converter.py input.json -o output.ts

作者: Claude
版本: 2.0.0
"""

import json
import re
import argparse
import sys
from pathlib import Path
from collections import defaultdict
from dataclasses import dataclass, field
from typing import Dict, List, Any, Optional, Tuple, Set
from enum import Enum


# ============================================================================
# 数据类定义
# ============================================================================

@dataclass
class ComponentInfo:
    """组件信息"""
    name: str
    safe_name: str
    component_type: str
    parent: str
    is_native_parent: bool
    children: List[str] = field(default_factory=list)


@dataclass
class PropertyInfo:
    """属性信息"""
    name: str
    safe_name: str
    ts_type: str
    serialized_type: str
    is_component: bool = False


@dataclass
class ParameterInfo:
    """函数参数"""
    name: str
    safe_name: str
    ts_type: str
    is_out: bool = False
    is_return: bool = False


@dataclass
class FunctionInfo:
    """函数信息"""
    name: str
    safe_name: str
    params: List[ParameterInfo]
    return_type: str
    bytecode: List[dict]
    decompiled_code: List[str] = field(default_factory=list)


@dataclass
class BlueprintInfo:
    """蓝图信息"""
    class_name: str
    safe_name: str
    parent_class: str
    folder_path: str
    components: Dict[str, ComponentInfo] = field(default_factory=dict)
    properties: List[PropertyInfo] = field(default_factory=list)
    functions: List[FunctionInfo] = field(default_factory=list)


# ============================================================================
# 类型映射
# ============================================================================

PROPERTY_TYPE_MAP = {
    'BoolProperty': 'boolean',
    'IntProperty': 'number',
    'Int64Property': 'bigint',
    'FloatProperty': 'number',
    'DoubleProperty': 'number',
    'StrProperty': 'string',
    'NameProperty': 'string',
    'TextProperty': 'string',
    'ByteProperty': 'number',
    'ArrayProperty': 'any[]',
    'MapProperty': 'Map<any, any>',
    'SetProperty': 'Set<any>',
}

PARENT_CLASS_MAP = {
    'Actor': 'UE.Actor',
    'Pawn': 'UE.Pawn',
    'Character': 'UE.Character',
    'PlayerController': 'UE.PlayerController',
    'AIController': 'UE.AIController',
    'GameModeBase': 'UE.GameModeBase',
    'GameMode': 'UE.GameMode',
    'PlayerState': 'UE.PlayerState',
    'HUD': 'UE.HUD',
    'UserWidget': 'UE.UserWidget',
    'ActorComponent': 'UE.ActorComponent',
    'SceneComponent': 'UE.SceneComponent',
    'AnimInstance': 'UE.AnimInstance',
}

# 二元运算符映射
BINARY_OPERATORS = {
    'Multiply_DoubleDouble': '*',
    'Multiply_FloatFloat': '*',
    'Multiply_VectorFloat': '*',
    'Multiply_VectorDouble': '*',
    'Multiply_VectorVector': '*',
    'Multiply_IntInt': '*',
    'Add_DoubleDouble': '+',
    'Add_FloatFloat': '+',
    'Add_VectorVector': '+',
    'Add_IntInt': '+',
    'Subtract_DoubleDouble': '-',
    'Subtract_FloatFloat': '-',
    'Subtract_VectorVector': '-',
    'Subtract_IntInt': '-',
    'Divide_DoubleDouble': '/',
    'Divide_FloatFloat': '/',
    'Divide_IntInt': '/',
    'Less_DoubleDouble': '<',
    'Less_FloatFloat': '<',
    'Less_IntInt': '<',
    'LessEqual_DoubleDouble': '<=',
    'LessEqual_IntInt': '<=',
    'Greater_DoubleDouble': '>',
    'Greater_FloatFloat': '>',
    'Greater_IntInt': '>',
    'GreaterEqual_DoubleDouble': '>=',
    'GreaterEqual_IntInt': '>=',
    'EqualEqual_DoubleDouble': '===',
    'EqualEqual_FloatFloat': '===',
    'EqualEqual_IntInt': '===',
    'EqualEqual_BoolBool': '===',
    'NotEqual_DoubleDouble': '!==',
    'NotEqual_IntInt': '!==',
    'NotEqual_BoolBool': '!==',
    'BooleanAND': '&&',
    'BooleanOR': '||',
    'And_IntInt': '&',
    'Or_IntInt': '|',
    'Xor_IntInt': '^',
}


# ============================================================================
# 工具函数
# ============================================================================

def sanitize_name(name: str) -> str:
    """将名称转换为有效的TypeScript标识符"""
    if not name:
        return '_unnamed'
    result = re.sub(r'[\[\]{}()]', '', str(name))
    result = re.sub(r'[^a-zA-Z0-9_]', '_', result)
    result = re.sub(r'^(\d)', r'_\1', result)
    result = re.sub(r'_+', '_', result)
    return result.strip('_') or '_unnamed'


# ============================================================================
# 蓝图解析器
# ============================================================================

class BlueprintParser:
    """蓝图JSON解析器"""
    
    def __init__(self, json_path: str):
        with open(json_path, 'r', encoding='utf-8') as f:
            self.data = json.load(f)
        
        self.imports: Dict[int, dict] = {}
        self._build_import_map()
    
    def _build_import_map(self):
        """构建Import索引映射"""
        for i, imp in enumerate(self.data.get('Imports', [])):
            self.imports[-(i + 1)] = imp
    
    def _resolve_import(self, index: int) -> Optional[dict]:
        """解析Import引用"""
        return self.imports.get(index)
    
    def _get_func_info(self, stack_node: int) -> Tuple[str, str]:
        """获取函数信息 (库名, 函数名)"""
        if stack_node and isinstance(stack_node, int) and stack_node < 0:
            imp = self._resolve_import(stack_node)
            if imp:
                outer = imp.get('OuterIndex', 0)
                outer_name = ''
                if isinstance(outer, int) and outer < 0:
                    outer_imp = self._resolve_import(outer)
                    if outer_imp:
                        outer_name = outer_imp.get('ObjectName', '')
                return outer_name, imp.get('ObjectName', '')
        return '', ''
    
    def _get_ts_type(self, prop: dict) -> str:
        """获取TypeScript类型"""
        serialized_type = prop.get('SerializedType', '')
        
        # 基本类型
        if serialized_type in PROPERTY_TYPE_MAP:
            return PROPERTY_TYPE_MAP[serialized_type]
        
        # Object类型
        if serialized_type == 'ObjectProperty':
            prop_class = prop.get('PropertyClass')
            if prop_class and prop_class < 0:
                imp = self._resolve_import(prop_class)
                if imp:
                    return f"UE.{imp.get('ObjectName', 'Object')}"
            return 'UE.Object'
        
        # Struct类型
        if serialized_type == 'StructProperty':
            struct = prop.get('Struct')
            if struct and struct < 0:
                imp = self._resolve_import(struct)
                if imp:
                    return f"UE.{imp.get('ObjectName', '')}"
            return 'any'
        
        return 'any'
    
    def parse(self) -> BlueprintInfo:
        """解析蓝图"""
        blueprint = BlueprintInfo(
            class_name='',
            safe_name='',
            parent_class='UE.Actor',
            folder_path=self.data.get('FolderName', ''),
        )
        
        exports = self.data.get('Exports', [])
        
        # 第一遍：解析SCS_Node组件
        self._parse_components(exports, blueprint)
        
        # 第二遍：解析类和函数
        for exp in exports:
            exp_type = exp.get('$type', '')
            
            if 'ClassExport' in exp_type:
                self._parse_class(exp, blueprint)
            elif 'FunctionExport' in exp_type:
                self._parse_function(exp, blueprint)
        
        return blueprint
    
    def _parse_components(self, exports: List[dict], blueprint: BlueprintInfo):
        """解析组件层级"""
        scs_nodes = {}  # export_index -> node_info
        
        for i, exp in enumerate(exports):
            class_idx = exp.get('ClassIndex', 0)
            if class_idx < 0:
                imp = self._resolve_import(class_idx)
                if imp and imp.get('ObjectName') == 'SCS_Node':
                    node_data = {}
                    for key in ['Data', 'ExtraData']:
                        if key in exp and isinstance(exp[key], list):
                            for item in exp[key]:
                                if isinstance(item, dict) and 'Name' in item:
                                    node_data[item['Name']] = item.get('Value', item.get('$value'))
                    
                    internal_name = node_data.get('InternalVariableName', '')
                    if not internal_name:
                        continue
                    
                    parent_name = node_data.get('ParentComponentOrVariableName', '')
                    is_native = node_data.get('bIsParentComponentNative', False)
                    
                    # 组件类型
                    comp_class_idx = node_data.get('ComponentClass', 0)
                    comp_type = 'SceneComponent'
                    if comp_class_idx and comp_class_idx < 0:
                        imp2 = self._resolve_import(comp_class_idx)
                        if imp2:
                            comp_type = imp2.get('ObjectName', 'SceneComponent')
                    
                    # 子节点索引
                    child_indices = []
                    child_nodes = node_data.get('ChildNodes', [])
                    if isinstance(child_nodes, list):
                        for child in child_nodes:
                            if isinstance(child, dict):
                                child_idx = child.get('Value', 0)
                                if child_idx:
                                    child_indices.append(child_idx)
                    
                    scs_nodes[i + 1] = {
                        'name': internal_name,
                        'type': comp_type,
                        'parent': parent_name,
                        'is_native': is_native,
                        'children': child_indices
                    }
        
        # 根据ChildNodes设置父子关系
        for exp_idx, node in scs_nodes.items():
            for child_idx in node['children']:
                if child_idx in scs_nodes:
                    child_node = scs_nodes[child_idx]
                    if not child_node['parent']:
                        child_node['parent'] = node['name']
                        child_node['is_native'] = False
        
        # 转换为ComponentInfo
        for exp_idx, node in scs_nodes.items():
            comp = ComponentInfo(
                name=node['name'],
                safe_name=sanitize_name(node['name']),
                component_type=node['type'],
                parent=node['parent'],
                is_native_parent=node['is_native'],
                children=[scs_nodes[ci]['name'] for ci in node['children'] if ci in scs_nodes]
            )
            blueprint.components[comp.safe_name] = comp
    
    def _parse_class(self, exp: dict, blueprint: BlueprintInfo):
        """解析类定义"""
        raw_name = exp.get('ObjectName', '').replace('_C', '')
        blueprint.class_name = raw_name
        blueprint.safe_name = sanitize_name(raw_name)
        
        # 父类
        super_idx = exp.get('SuperIndex', 0)
        if super_idx < 0:
            imp = self._resolve_import(super_idx)
            if imp:
                parent = imp.get('ObjectName', 'Actor')
                blueprint.parent_class = PARENT_CLASS_MAP.get(parent, f'UE.{parent}')
        
        # 属性
        comp_names = set(blueprint.components.keys())
        for prop in exp.get('LoadedProperties', []):
            prop_name = prop.get('Name', '')
            if prop_name.startswith('UberGraph'):
                continue
            
            safe_name = sanitize_name(prop_name)
            is_component = safe_name in comp_names
            
            blueprint.properties.append(PropertyInfo(
                name=prop_name,
                safe_name=safe_name,
                ts_type=self._get_ts_type(prop),
                serialized_type=prop.get('SerializedType', ''),
                is_component=is_component
            ))
    
    def _parse_function(self, exp: dict, blueprint: BlueprintInfo):
        """解析函数"""
        func_name = exp.get('ObjectName', '')
        if func_name.startswith('ExecuteUbergraph'):
            return
        
        params = []
        return_type = 'void'
        
        for prop in exp.get('LoadedProperties', []):
            prop_name = prop.get('Name', '')
            prop_flags = prop.get('PropertyFlags', '')
            
            # 跳过临时变量
            if prop_name.startswith('CallFunc_') or prop_name.startswith('K2Node_'):
                continue
            if prop_name.startswith('Temp_'):
                continue
            
            ts_type = self._get_ts_type(prop)
            
            if 'CPF_ReturnParm' in prop_flags:
                return_type = ts_type
            elif 'CPF_Parm' in prop_flags:
                is_out = 'CPF_OutParm' in prop_flags
                params.append(ParameterInfo(
                    name=prop_name,
                    safe_name=sanitize_name(prop_name),
                    ts_type=ts_type,
                    is_out=is_out
                ))
        
        blueprint.functions.append(FunctionInfo(
            name=func_name,
            safe_name=sanitize_name(func_name),
            params=params,
            return_type=return_type,
            bytecode=exp.get('ScriptBytecode', [])
        ))


# ============================================================================
# 字节码反编译器
# ============================================================================

class BytecodeDecompiler:
    """字节码反编译器"""
    
    def __init__(self, imports: Dict[int, dict]):
        self.imports = imports
        self.var_values: Dict[str, str] = {}  # 变量名 -> 表达式值
        self.indent = '        '
    
    def _resolve_import(self, index: int) -> Optional[dict]:
        return self.imports.get(index)
    
    def _get_func_info(self, stack_node: int) -> Tuple[str, str]:
        if stack_node and isinstance(stack_node, int) and stack_node < 0:
            imp = self._resolve_import(stack_node)
            if imp:
                outer = imp.get('OuterIndex', 0)
                outer_name = ''
                if isinstance(outer, int) and outer < 0:
                    outer_imp = self._resolve_import(outer)
                    if outer_imp:
                        outer_name = outer_imp.get('ObjectName', '')
                return outer_name, imp.get('ObjectName', '')
        return '', ''
    
    def _get_var_path(self, var_info: dict) -> Optional[str]:
        if not var_info:
            return None
        path = var_info.get('New', {}).get('Path', [])
        return path[0] if path else None
    
    def _simplify_var_name(self, name: str) -> str:
        """简化变量名"""
        if not name:
            return '_temp'
        
        # 移除CallFunc_前缀和_ReturnValue后缀
        if name.startswith('CallFunc_'):
            name = name[9:]
        name = re.sub(r'_ReturnValue.*$', '', name)
        
        # 移除末尾数字
        name = re.sub(r'_\d+$', '', name)
        
        return sanitize_name(name)
    
    def _expr(self, e: dict, depth: int = 0) -> str:
        """解析表达式"""
        if not e or depth > 15:
            return 'null'
        
        t = e.get('$type', '').split('.')[-1].replace(', UAssetAPI', '')
        
        # 实例变量
        if t == 'EX_InstanceVariable':
            v = self._get_var_path(e.get('Variable', {}))
            return f'this.{sanitize_name(v)}' if v else 'this._temp'
        
        # 局部变量
        if t in ('EX_LocalVariable', 'EX_LocalOutVariable'):
            name = self._get_var_path(e.get('Variable', {}))
            if name:
                simple = self._simplify_var_name(name)
                # 如果是已计算的值，返回该值
                if name in self.var_values:
                    return self.var_values[name]
                return simple
            return '_temp'
        
        # 常量
        if t == 'EX_DoubleConst' or t == 'EX_FloatConst':
            v = e.get('Value', 0)
            try:
                v = float(v)
                return f'{v}' if v >= 0 else f'({v})'
            except:
                return str(v)
        
        if t == 'EX_IntConst':
            return str(e.get('Value', 0))
        
        if t == 'EX_ByteConst':
            return str(e.get('Value', 0))
        
        if t == 'EX_True':
            return 'true'
        
        if t == 'EX_False':
            return 'false'
        
        if t == 'EX_Self':
            return 'this'
        
        if t == 'EX_NoObject':
            return 'null'
        
        if t == 'EX_VectorConst':
            v = e.get('Value', {})
            x, y, z = v.get('X', 0), v.get('Y', 0), v.get('Z', 0)
            return f'new UE.Vector({x}, {y}, {z})'
        
        if t == 'EX_RotatorConst':
            v = e.get('Value', {})
            p, y, r = v.get('Pitch', 0), v.get('Yaw', 0), v.get('Roll', 0)
            return f'new UE.Rotator({p}, {y}, {r})'
        
        if t == 'EX_NameConst':
            return f'"{e.get("Value", "")}"'
        
        if t == 'EX_StringConst':
            return f'"{e.get("Value", "")}"'
        
        # 数学调用
        if t == 'EX_CallMath':
            outer, func = self._get_func_info(e.get('StackNode', 0))
            params = e.get('Parameters', [])
            ps = [self._expr(p, depth + 1) for p in params]
            
            # 二元运算符
            if func in BINARY_OPERATORS and len(ps) >= 2:
                op = BINARY_OPERATORS[func]
                return f'({ps[0]} {op} {ps[1]})'
            
            # 特殊函数
            if func == 'Conv_RotatorToVector' and ps:
                return f'{ps[0]}.Vector()'
            if func == 'Vector_NormalUnsafe' and ps:
                return f'{ps[0]}.GetUnsafeNormal()'
            if func == 'MakeVector' and len(ps) >= 3:
                return f'new UE.Vector({ps[0]}, {ps[1]}, {ps[2]})'
            if func == 'MakeVector2D' and len(ps) >= 2:
                return f'new UE.Vector2D({ps[0]}, {ps[1]})'
            if func == 'MakeRotator' and len(ps) >= 3:
                return f'new UE.Rotator({ps[0]}, {ps[1]}, {ps[2]})'
            if func == 'SelectFloat' and len(ps) >= 3:
                return f'({ps[2]} ? {ps[0]} : {ps[1]})'
            if func == 'SelectInt' and len(ps) >= 3:
                return f'({ps[2]} ? {ps[0]} : {ps[1]})'
            if func == 'SelectVector' and len(ps) >= 3:
                return f'({ps[2]} ? {ps[0]} : {ps[1]})'
            if func == 'Not_PreBool' and ps:
                return f'!{ps[0]}'
            if func == 'FInterpTo' and len(ps) >= 4:
                return f'UE.KismetMathLibrary.FInterpTo({ps[0]}, {ps[1]}, {ps[2]}, {ps[3]})'
            if func == 'VInterpTo' and len(ps) >= 4:
                return f'UE.KismetMathLibrary.VInterpTo({ps[0]}, {ps[1]}, {ps[2]}, {ps[3]})'
            if func == 'RInterpTo' and len(ps) >= 4:
                return f'UE.KismetMathLibrary.RInterpTo({ps[0]}, {ps[1]}, {ps[2]}, {ps[3]})'
            if func == 'RandomFloatInRange' and len(ps) >= 2:
                return f'UE.KismetMathLibrary.RandomFloatInRange({ps[0]}, {ps[1]})'
            if func == 'RandomIntegerInRange' and len(ps) >= 2:
                return f'UE.KismetMathLibrary.RandomIntegerInRange({ps[0]}, {ps[1]})'
            if func == 'Clamp' and len(ps) >= 3:
                return f'UE.KismetMathLibrary.Clamp({ps[0]}, {ps[1]}, {ps[2]})'
            if func == 'FClamp' and len(ps) >= 3:
                return f'UE.KismetMathLibrary.FClamp({ps[0]}, {ps[1]}, {ps[2]})'
            if func == 'Abs' and ps:
                return f'Math.abs({ps[0]})'
            if func == 'Sqrt' and ps:
                return f'Math.sqrt({ps[0]})'
            if func == 'Sin' and ps:
                return f'Math.sin({ps[0]})'
            if func == 'Cos' and ps:
                return f'Math.cos({ps[0]})'
            if func == 'K2_GetActorLocation':
                return 'this.K2_GetActorLocation()'
            if func == 'K2_GetActorRotation':
                return 'this.K2_GetActorRotation()'
            if func == 'GetActorForwardVector':
                return 'this.GetActorForwardVector()'
            if func == 'GetActorRightVector':
                return 'this.GetActorRightVector()'
            if func == 'GetActorUpVector':
                return 'this.GetActorUpVector()'
            if func == 'Array_Length' and ps:
                return f'{ps[0]}.length'
            if func == 'Array_Get' and len(ps) >= 2:
                return f'{ps[0]}[{ps[1]}]'
            
            # 通用调用
            if outer:
                return f'UE.{outer}.{func}({", ".join(ps)})'
            return f'{func}({", ".join(ps)})'
        
        # 最终函数调用
        if t == 'EX_FinalFunction' or t == 'EX_LocalFinalFunction':
            outer, func = self._get_func_info(e.get('StackNode', 0))
            params = e.get('Parameters', [])
            ps = [self._expr(p, depth + 1) for p in params]
            return f'this.{func}({", ".join(ps)})' if func else 'null'
        
        # 虚函数调用
        if t == 'EX_VirtualFunction' or t == 'EX_LocalVirtualFunction':
            func = e.get('VirtualFunctionName', '')
            params = e.get('Parameters', [])
            ps = [self._expr(p, depth + 1) for p in params]
            return f'this.{func}({", ".join(ps)})' if func else 'null'
        
        # 类型转换
        if t == 'EX_PrimitiveCast':
            return self._expr(e.get('Target', {}), depth + 1)
        
        # 上下文
        if t == 'EX_Context':
            obj = self._expr(e.get('ObjectExpression', {}), depth + 1)
            ctx = e.get('ContextExpression', {})
            if ctx:
                return f'{obj}.{self._expr(ctx, depth + 1)}'
            return obj
        
        # 结构体成员
        if t == 'EX_StructMemberContext':
            prop = e.get('StructMemberExpression', {})
            path = prop.get('New', {}).get('Path', [])
            member = path[0] if path else ''
            struct_expr = self._expr(e.get('StructExpression', {}), depth + 1)
            return f'{struct_expr}.{member}'
        
        return f'/* {t} */'
    
    def decompile(self, bytecode: List[dict]) -> List[str]:
        """反编译字节码"""
        self.var_values.clear()
        lines = []
        
        i = 0
        while i < len(bytecode):
            inst = bytecode[i]
            t = inst.get('$type', '').split('.')[-1].replace(', UAssetAPI', '')
            
            # 赋值
            if t in ('EX_Let', 'EX_LetBool', 'EX_LetObj', 'EX_LetDelegate',
                     'EX_LetMulticastDelegate', 'EX_LetWeakObjPtr'):
                var_info = inst.get('Value', {})
                var_name = self._get_var_path(var_info)
                
                if not var_name:
                    i += 1
                    continue
                
                # 跳过K2Node临时变量的显式赋值
                if var_name.startswith('K2Node_'):
                    i += 1
                    continue
                
                expr_val = self._expr(inst.get('Expression', {}))
                simple_name = self._simplify_var_name(var_name)
                
                # 存储变量值用于后续内联
                self.var_values[var_name] = expr_val
                
                # 判断是临时变量还是实际变量
                if var_name.startswith('CallFunc_') or var_name.startswith('Temp_'):
                    lines.append(f'{self.indent}const {simple_name} = {expr_val};')
                else:
                    # 检查是否是实例变量
                    if any(c.isalpha() and c.isupper() for c in var_name[:3]):
                        # 可能是实际变量
                        lines.append(f'{self.indent}{simple_name} = {expr_val};')
                    else:
                        lines.append(f'{self.indent}{simple_name} = {expr_val};')
            
            # BreakVector调用
            elif t == 'EX_CallMath':
                outer, func = self._get_func_info(inst.get('StackNode', 0))
                params = inst.get('Parameters', [])
                
                if func == 'BreakVector' and len(params) >= 4:
                    vec = self._expr(params[0])
                    x_name = self._get_var_path(params[1].get('Variable', {}))
                    y_name = self._get_var_path(params[2].get('Variable', {}))
                    z_name = self._get_var_path(params[3].get('Variable', {}))
                    
                    x = self._simplify_var_name(x_name) if x_name else '_x'
                    y = self._simplify_var_name(y_name) if y_name else '_y'
                    z = self._simplify_var_name(z_name) if z_name else '_z'
                    
                    lines.append(f'{self.indent}const [{x}, {y}, {z}] = [{vec}.X, {vec}.Y, {vec}.Z];')
                    
                    # 存储变量值
                    if x_name:
                        self.var_values[x_name] = f'{vec}.X'
                    if y_name:
                        self.var_values[y_name] = f'{vec}.Y'
                    if z_name:
                        self.var_values[z_name] = f'{vec}.Z'
                
                elif func == 'BreakVector2D' and len(params) >= 3:
                    vec = self._expr(params[0])
                    x_name = self._get_var_path(params[1].get('Variable', {}))
                    y_name = self._get_var_path(params[2].get('Variable', {}))
                    
                    x = self._simplify_var_name(x_name) if x_name else '_x'
                    y = self._simplify_var_name(y_name) if y_name else '_y'
                    
                    lines.append(f'{self.indent}const [{x}, {y}] = [{vec}.X, {vec}.Y];')
                
                elif func == 'BreakRotator' and len(params) >= 4:
                    rot = self._expr(params[0])
                    r_name = self._get_var_path(params[1].get('Variable', {}))
                    p_name = self._get_var_path(params[2].get('Variable', {}))
                    y_name = self._get_var_path(params[3].get('Variable', {}))
                    
                    r = self._simplify_var_name(r_name) if r_name else '_roll'
                    p = self._simplify_var_name(p_name) if p_name else '_pitch'
                    y = self._simplify_var_name(y_name) if y_name else '_yaw'
                    
                    lines.append(f'{self.indent}const [{r}, {p}, {y}] = [{rot}.Roll, {rot}.Pitch, {rot}.Yaw];')
                
                elif func.startswith('LineTrace'):
                    ps = [self._expr(p) for p in params[:5]]
                    lines.append(f'{self.indent}// {func}(...)')
                
                elif func.startswith('BreakHitResult'):
                    lines.append(f'{self.indent}// BreakHitResult(...)')
            
            # 函数调用 (无返回值)
            elif t in ('EX_LocalFinalFunction', 'EX_FinalFunction', 
                       'EX_VirtualFunction', 'EX_LocalVirtualFunction'):
                outer, func = self._get_func_info(inst.get('StackNode', 0))
                
                if t in ('EX_VirtualFunction', 'EX_LocalVirtualFunction'):
                    func = inst.get('VirtualFunctionName', '')
                
                if func:
                    params = inst.get('Parameters', [])
                    ps = [self._expr(p) for p in params]
                    lines.append(f'{self.indent}this.{func}({", ".join(ps)});')
            
            # 条件跳转
            elif t == 'EX_JumpIfNot':
                cond = self._expr(inst.get('BooleanExpression', {}))
                # 简化: 只添加注释
                lines.append(f'{self.indent}// if (!({cond})) {{ ... }}')
            
            # 跳转
            elif t == 'EX_Jump':
                pass  # 通常是循环或分支的一部分
            
            # 返回
            elif t == 'EX_Return':
                ret_prop = inst.get('ReturnExpression', {})
                if ret_prop:
                    ret_val = self._expr(ret_prop)
                    if ret_val != 'null':
                        lines.append(f'{self.indent}return {ret_val};')
            
            i += 1
        
        return lines


# ============================================================================
# 代码生成器
# ============================================================================

class CodeGenerator:
    """TypeScript代码生成器"""
    
    def __init__(self, blueprint: BlueprintInfo, imports: Dict[int, dict]):
        self.bp = blueprint
        self.decompiler = BytecodeDecompiler(imports)
    
    def generate(self) -> str:
        """生成TypeScript代码"""
        lines = []
        
        # 文件头
        lines.extend(self._gen_header())
        
        # 导入
        lines.extend(self._gen_imports())
        
        # 类定义
        lines.extend(self._gen_class())
        
        # 导出
        lines.append('')
        lines.append(f'export default {self.bp.safe_name};')
        
        return '\n'.join(lines)
    
    def _gen_header(self) -> List[str]:
        return [
            '/**',
            f' * {self.bp.safe_name}',
            ' * 使用装饰器简化组件创建',
            ' */',
            '',
        ]
    
    def _gen_imports(self) -> List[str]:
        return [
            "import * as UE from 'ue'",
            "import {uproperty} from 'ue'",
            '',
            "import './ObjectExt'",
            '',
        ]
    
    def _gen_class(self) -> List[str]:
        lines = []
        
        lines.append(f'class {self.bp.safe_name} extends {self.bp.parent_class} {{')
        lines.append('')
        
        # 组件层级注释
        lines.extend(self._gen_component_hierarchy_comment())
        
        # 组件声明
        lines.extend(self._gen_components())
        
        # 变量
        lines.extend(self._gen_variables())
        
        # 构造函数
        lines.extend(self._gen_constructor())
        
        # 函数
        lines.extend(self._gen_functions())
        
        # 事件
        lines.extend(self._gen_events())
        
        lines.append('}')
        
        return lines
    
    def _gen_component_hierarchy_comment(self) -> List[str]:
        lines = [
            '    // ============================================================',
            '    // 组件层级结构',
            '    // ============================================================',
            '    //',
            f'    // {self.bp.parent_class.replace("UE.", "")} (Self)',
            '    // └── CapsuleComponent [原生]',
        ]
        
        # 分组组件
        native_comps = []
        bp_children = defaultdict(list)
        
        for comp in self.bp.components.values():
            if comp.is_native_parent:
                native_comps.append(comp)
            elif comp.parent:
                bp_children[sanitize_name(comp.parent)].append(comp)
        
        for comp in native_comps:
            children = bp_children.get(comp.safe_name, [])
            if children:
                lines.append(f'    //     ├── {comp.safe_name}')
                for i, c in enumerate(children):
                    prefix = '└──' if i == len(children) - 1 else '├──'
                    lines.append(f'    //     │   {prefix} {c.safe_name}')
            else:
                lines.append(f'    //     ├── {comp.safe_name}')
        
        lines.append('')
        return lines
    
    def _gen_components(self) -> List[str]:
        lines = []
        
        # 分组
        native_comps = []
        bp_children = defaultdict(list)
        independent = []
        
        for comp in self.bp.components.values():
            if comp.is_native_parent:
                native_comps.append(comp)
            elif comp.parent:
                bp_children[sanitize_name(comp.parent)].append(comp)
            else:
                if comp.safe_name != 'DefaultSceneRoot':
                    independent.append(comp)
        
        # 第一层
        if native_comps:
            lines.extend([
                '    // ============================================================',
                '    // 第一层: 附加到 CapsuleComponent',
                '    // ============================================================',
                '',
            ])
            for comp in native_comps:
                lines.append(f'    @uproperty.attach("CapsuleComponent")')
                lines.append(f'    {comp.safe_name}: UE.{comp.component_type};')
                lines.append('')
        
        # 第二层
        for parent_name, children in bp_children.items():
            if children:
                lines.extend([
                    '    // ============================================================',
                    f'    // 第二层: 附加到 {parent_name}',
                    '    // ============================================================',
                    '',
                ])
                for comp in children:
                    lines.append(f'    @uproperty.attach("{parent_name}")')
                    lines.append(f'    {comp.safe_name}: UE.{comp.component_type};')
                    lines.append('')
        
        return lines
    
    def _gen_variables(self) -> List[str]:
        # 过滤掉组件
        var_props = [p for p in self.bp.properties if not p.is_component]
        
        if not var_props:
            return []
        
        lines = [
            '    // ============================================================',
            '    // 变量',
            '    // ============================================================',
            '',
        ]
        
        for prop in var_props:
            lines.append(f'    {prop.safe_name}: {prop.ts_type};')
        
        lines.append('')
        return lines
    
    def _gen_constructor(self) -> List[str]:
        return [
            '    // ============================================================',
            '    // 构造函数',
            '    // ============================================================',
            '',
            '    Constructor() {',
            '        // 组件已由装饰器自动创建和附加',
            '    }',
            '',
        ]
    
    def _gen_functions(self) -> List[str]:
        if not self.bp.functions:
            return []
        
        lines = [
            '    // ============================================================',
            '    // 函数',
            '    // ============================================================',
        ]
        
        for func in self.bp.functions:
            lines.append('')
            
            # 签名
            params_str = ', '.join(
                f'{"/* out */ " if p.is_out else ""}{p.safe_name}: {p.ts_type}'
                for p in func.params
            )
            
            lines.append(f'    {func.safe_name}({params_str}): {func.return_type} {{')
            
            # 反编译函数体
            if func.bytecode:
                decompiled = self.decompiler.decompile(func.bytecode)
                if decompiled:
                    lines.extend(decompiled)
                else:
                    lines.append('        // TODO: 实现')
            else:
                lines.append('        // TODO: 实现')
            
            lines.append('    }')
        
        lines.append('')
        return lines
    
    def _gen_events(self) -> List[str]:
        return [
            '    // ============================================================',
            '    // 事件',
            '    // ============================================================',
            '',
            '    ReceiveBeginPlay(): void {',
            '',
            '    }',
            '',
            '    ReceiveTick(DeltaSeconds: number): void {',
            '        // 每帧更新',
            '    }',
        ]


# ============================================================================
# 主转换函数
# ============================================================================

def convert_blueprint(json_path: str, output_path: str = None) -> str:
    """
    转换蓝图JSON到PuerTS TypeScript
    
    Args:
        json_path: 输入JSON文件路径
        output_path: 输出TS文件路径 (可选)
    
    Returns:
        生成的TypeScript代码
    """
    parser = BlueprintParser(json_path)
    blueprint = parser.parse()
    
    generator = CodeGenerator(blueprint, parser.imports)
    code = generator.generate()
    
    if output_path:
        Path(output_path).write_text(code, encoding='utf-8')
        print(f'✓ 转换完成: {output_path}')
        print(f'  类名: {blueprint.class_name}')
        print(f'  父类: {blueprint.parent_class}')
        print(f'  组件: {len(blueprint.components)}')
        print(f'  变量: {len([p for p in blueprint.properties if not p.is_component])}')
        print(f'  函数: {len(blueprint.functions)}')
    
    return code


# ============================================================================
# 命令行入口
# ============================================================================

def main():
    parser = argparse.ArgumentParser(
        description='Blueprint JSON to PuerTS TypeScript 转换工具'
    )
    parser.add_argument('input', help='输入的JSON文件')
    parser.add_argument('-o', '--output', help='输出的TS文件')
    
    args = parser.parse_args()
    
    input_path = args.input
    output_path = args.output
    
    if not output_path:
        output_path = Path(input_path).stem + '.ts'
    
    convert_blueprint(input_path, output_path)


if __name__ == '__main__':
    main()
