#!/usr/bin/env python3
"""
bp2puerts_cli.py - Blueprint to PuerTs 命令行工具

支持功能:
- 单文件转换
- 批量转换
- 配置文件
- 详细输出
- 统计信息

使用方法:
    # 基本使用
    python bp2puerts_cli.py input.json
    
    # 指定输出
    python bp2puerts_cli.py input.json -o output.ts
    
    # 批量转换
    python bp2puerts_cli.py *.json -d output_dir/
    
    # 使用配置文件
    python bp2puerts_cli.py input.json -c config.json
    
    # 详细输出
    python bp2puerts_cli.py input.json -v

作者: Claude
版本: 1.0.0
"""

import argparse
import json
import sys
import time
from pathlib import Path
from typing import List, Optional
from dataclasses import asdict

# 导入核心模块
try:
    from bp2puerts_core import (
        BlueprintConverter,
        ConvertConfig,
        ConvertResult,
        convert_file,
        __version__,
    )
except ImportError:
    # 如果在同目录下
    import os
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from bp2puerts_core import (
        BlueprintConverter,
        ConvertConfig,
        ConvertResult,
        convert_file,
        __version__,
    )


# ============================================
# 颜色输出
# ============================================

class Colors:
    """终端颜色"""
    RESET = '\033[0m'
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    MAGENTA = '\033[95m'
    CYAN = '\033[96m'
    BOLD = '\033[1m'
    
    @classmethod
    def disable(cls):
        """禁用颜色"""
        cls.RESET = ''
        cls.RED = ''
        cls.GREEN = ''
        cls.YELLOW = ''
        cls.BLUE = ''
        cls.MAGENTA = ''
        cls.CYAN = ''
        cls.BOLD = ''


def print_success(msg: str):
    print(f"{Colors.GREEN}✓{Colors.RESET} {msg}")


def print_error(msg: str):
    print(f"{Colors.RED}✗{Colors.RESET} {msg}", file=sys.stderr)


def print_warning(msg: str):
    print(f"{Colors.YELLOW}⚠{Colors.RESET} {msg}")


def print_info(msg: str):
    print(f"{Colors.BLUE}ℹ{Colors.RESET} {msg}")


def print_header(msg: str):
    print(f"\n{Colors.BOLD}{Colors.CYAN}{msg}{Colors.RESET}")


# ============================================
# 配置加载
# ============================================

def load_config(config_path: str) -> ConvertConfig:
    """从JSON文件加载配置"""
    with open(config_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    
    config = ConvertConfig()
    
    # 映射配置字段
    field_map = {
        'include_comments': 'include_comments',
        'include_bytecode_hints': 'include_bytecode_hints',
        'max_properties': 'max_properties',
        'max_functions': 'max_functions',
        'indent': 'indent',
        'use_async_delay': 'use_async_delay',
        'generate_helper_functions': 'generate_helper_functions',
        'infer_enum_types': 'infer_enum_types',
        'use_strict_types': 'use_strict_types',
        'group_properties': 'group_properties',
        'group_functions': 'group_functions',
        'skip_internal_properties': 'skip_internal_properties',
        'skip_temp_variables': 'skip_temp_variables',
    }
    
    for json_key, attr_name in field_map.items():
        if json_key in data:
            setattr(config, attr_name, data[json_key])
    
    return config


def save_default_config(output_path: str):
    """保存默认配置到文件"""
    config = ConvertConfig()
    
    config_dict = {
        'include_comments': config.include_comments,
        'include_bytecode_hints': config.include_bytecode_hints,
        'max_properties': config.max_properties,
        'max_functions': config.max_functions,
        'indent': config.indent,
        'use_async_delay': config.use_async_delay,
        'generate_helper_functions': config.generate_helper_functions,
        'infer_enum_types': config.infer_enum_types,
        'use_strict_types': config.use_strict_types,
        'group_properties': config.group_properties,
        'group_functions': config.group_functions,
        'skip_internal_properties': config.skip_internal_properties,
        'skip_temp_variables': config.skip_temp_variables,
    }
    
    with open(output_path, 'w', encoding='utf-8') as f:
        json.dump(config_dict, f, indent=2, ensure_ascii=False)
    
    print_success(f"默认配置已保存到: {output_path}")


# ============================================
# 转换函数
# ============================================

def convert_single(
    input_path: str,
    output_path: Optional[str],
    config: ConvertConfig,
    verbose: bool = False,
    preview: bool = False,
) -> bool:
    """转换单个文件"""
    
    input_file = Path(input_path)
    
    if not input_file.exists():
        print_error(f"文件不存在: {input_path}")
        return False
    
    # 确定输出路径
    if output_path is None:
        output_path = str(input_file.with_suffix('.ts'))
    
    if verbose:
        print_info(f"输入: {input_path}")
        print_info(f"输出: {output_path}")
    
    # 执行转换
    start_time = time.time()
    
    try:
        converter = BlueprintConverter(input_path, config)
        result = converter.convert()
    except Exception as e:
        print_error(f"转换失败: {e}")
        return False
    
    elapsed = time.time() - start_time
    
    if not result.success:
        print_error(f"转换失败: {input_path}")
        for error in result.errors:
            print_error(f"  {error}")
        return False
    
    # 显示警告
    for warning in result.warnings:
        print_warning(warning)
    
    # 预览模式
    if preview:
        print_header("预览输出:")
        print(result.code[:2000])
        if len(result.code) > 2000:
            print(f"\n... (还有 {len(result.code) - 2000} 字符)")
        return True
    
    # 保存文件
    Path(output_path).write_text(result.code, encoding='utf-8')
    
    # 输出结果
    if verbose:
        print_header("转换统计:")
        print(f"  类名: {result.blueprint.class_name}")
        print(f"  父类: {result.blueprint.parent_class}")
        print(f"  属性: {result.stats.get('properties', 0)} 个")
        print(f"  组件: {result.stats.get('components', 0)} 个")
        print(f"  函数: {result.stats.get('functions', 0)} 个")
        print(f"  事件: {result.stats.get('events', 0)} 个")
        print(f"  代码: {result.stats.get('lines', 0)} 行")
        print(f"  耗时: {elapsed:.2f} 秒")
    
    print_success(f"转换完成: {input_path} -> {output_path}")
    return True


def convert_batch(
    input_patterns: List[str],
    output_dir: str,
    config: ConvertConfig,
    verbose: bool = False,
) -> tuple:
    """批量转换"""
    
    from glob import glob
    
    # 收集所有输入文件
    input_files = []
    for pattern in input_patterns:
        matches = glob(pattern)
        input_files.extend(matches)
    
    if not input_files:
        print_error("没有找到匹配的文件")
        return 0, 0
    
    # 创建输出目录
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)
    
    print_header(f"批量转换: {len(input_files)} 个文件")
    
    success_count = 0
    fail_count = 0
    
    for input_file in input_files:
        input_path = Path(input_file)
        output_file = output_path / input_path.with_suffix('.ts').name
        
        if convert_single(input_file, str(output_file), config, verbose=False):
            success_count += 1
        else:
            fail_count += 1
    
    print_header("批量转换完成")
    print_success(f"成功: {success_count} 个")
    if fail_count > 0:
        print_error(f"失败: {fail_count} 个")
    
    return success_count, fail_count


def show_info(input_path: str):
    """显示蓝图信息（不转换）"""
    
    try:
        converter = BlueprintConverter(input_path)
        bp = converter.parse()
    except Exception as e:
        print_error(f"解析失败: {e}")
        return
    
    print_header(f"蓝图信息: {input_path}")
    print(f"  类名: {Colors.CYAN}{bp.class_name}{Colors.RESET}")
    print(f"  原始名: {bp.original_name}")
    print(f"  父类: {Colors.MAGENTA}{bp.parent_class}{Colors.RESET}")
    print(f"  路径: {bp.folder_name}")
    
    # 组件
    components = [p for p in bp.properties if p.category.name == 'COMPONENT']
    if components:
        print_header("组件:")
        for comp in components[:10]:
            print(f"  - {comp.name}: {comp.type}")
        if len(components) > 10:
            print(f"  ... 还有 {len(components) - 10} 个")
    
    # 属性统计
    print_header("属性统计:")
    from collections import Counter
    categories = Counter(p.category.name for p in bp.properties)
    for cat, count in categories.items():
        print(f"  {cat}: {count}")
    
    # 函数
    print_header("函数:")
    events = [f for f in bp.functions if f.category.name == 'EVENT']
    others = [f for f in bp.functions if f.category.name != 'EVENT']
    
    print(f"  事件: {len(events)} 个")
    for func in events[:5]:
        print(f"    - {func.name}")
    
    print(f"  其他: {len(others)} 个")
    for func in others[:5]:
        print(f"    - {func.name}")
    if len(others) > 5:
        print(f"    ... 还有 {len(others) - 5} 个")


# ============================================
# 命令行参数
# ============================================

def create_parser() -> argparse.ArgumentParser:
    """创建参数解析器"""
    
    parser = argparse.ArgumentParser(
        prog='bp2puerts',
        description='Blueprint JSON to PuerTs TypeScript 转换工具',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  %(prog)s BP_Player.json                    # 转换单个文件
  %(prog)s BP_Player.json -o TS_Player.ts    # 指定输出文件
  %(prog)s *.json -d output/                 # 批量转换到目录
  %(prog)s BP_Player.json -v                 # 详细输出
  %(prog)s BP_Player.json --info             # 只显示信息
  %(prog)s BP_Player.json --preview          # 预览输出
  %(prog)s --init-config config.json         # 生成配置模板
        """
    )
    
    # 位置参数
    parser.add_argument(
        'input',
        nargs='*',
        help='输入的JSON文件 (支持glob模式)',
    )
    
    # 输出选项
    output_group = parser.add_argument_group('输出选项')
    output_group.add_argument(
        '-o', '--output',
        help='输出文件路径',
    )
    output_group.add_argument(
        '-d', '--output-dir',
        help='输出目录 (批量转换时使用)',
    )
    
    # 配置选项
    config_group = parser.add_argument_group('配置选项')
    config_group.add_argument(
        '-c', '--config',
        help='配置文件路径',
    )
    config_group.add_argument(
        '--init-config',
        metavar='PATH',
        help='生成默认配置文件',
    )
    
    # 转换选项
    convert_group = parser.add_argument_group('转换选项')
    convert_group.add_argument(
        '--no-comments',
        action='store_true',
        help='不生成注释',
    )
    convert_group.add_argument(
        '--no-bytecode',
        action='store_true',
        help='不包含字节码提示',
    )
    convert_group.add_argument(
        '--no-helpers',
        action='store_true',
        help='不生成辅助函数',
    )
    convert_group.add_argument(
        '--flat',
        action='store_true',
        help='不分组属性和函数',
    )
    convert_group.add_argument(
        '--max-props',
        type=int,
        metavar='N',
        help='最大属性数量',
    )
    convert_group.add_argument(
        '--max-funcs',
        type=int,
        metavar='N',
        help='最大函数数量',
    )
    
    # 显示选项
    display_group = parser.add_argument_group('显示选项')
    display_group.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='详细输出',
    )
    display_group.add_argument(
        '--info',
        action='store_true',
        help='只显示蓝图信息，不转换',
    )
    display_group.add_argument(
        '--preview',
        action='store_true',
        help='预览输出，不保存文件',
    )
    display_group.add_argument(
        '--no-color',
        action='store_true',
        help='禁用彩色输出',
    )
    
    # 其他
    parser.add_argument(
        '-V', '--version',
        action='version',
        version=f'%(prog)s {__version__}',
    )
    
    return parser


def build_config(args: argparse.Namespace) -> ConvertConfig:
    """从命令行参数构建配置"""
    
    # 基础配置
    if args.config:
        config = load_config(args.config)
    else:
        config = ConvertConfig()
    
    # 命令行覆盖
    if args.no_comments:
        config.include_comments = False
    
    if args.no_bytecode:
        config.include_bytecode_hints = False
    
    if args.no_helpers:
        config.generate_helper_functions = False
    
    if args.flat:
        config.group_properties = False
        config.group_functions = False
    
    if args.max_props is not None:
        config.max_properties = args.max_props
    
    if args.max_funcs is not None:
        config.max_functions = args.max_funcs
    
    return config


# ============================================
# 主函数
# ============================================

def main():
    """主入口"""
    
    parser = create_parser()
    args = parser.parse_args()
    
    # 禁用颜色
    if args.no_color:
        Colors.disable()
    
    # 生成配置文件
    if args.init_config:
        save_default_config(args.init_config)
        return 0
    
    # 检查输入
    if not args.input:
        parser.print_help()
        return 1
    
    # 构建配置
    config = build_config(args)
    
    # 显示信息模式
    if args.info:
        for input_file in args.input:
            show_info(input_file)
        return 0
    
    # 批量转换
    if args.output_dir:
        success, fail = convert_batch(
            args.input,
            args.output_dir,
            config,
            args.verbose,
        )
        return 0 if fail == 0 else 1
    
    # 单文件转换
    if len(args.input) == 1:
        success = convert_single(
            args.input[0],
            args.output,
            config,
            args.verbose,
            args.preview,
        )
        return 0 if success else 1
    
    # 多文件但没指定输出目录
    print_error("多个输入文件时请使用 -d 指定输出目录")
    return 1


if __name__ == '__main__':
    sys.exit(main())