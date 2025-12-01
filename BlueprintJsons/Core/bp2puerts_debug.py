#!/usr/bin/env python3
"""
bp2puerts 调试脚本 - 用于诊断启动问题
"""

import sys
print(f"Python版本: {sys.version}")
print(f"Python路径: {sys.executable}")
print()

# 检查tkinter
print("=== 检查 tkinter ===")
try:
    import tkinter as tk
    print(f"✓ tkinter 导入成功")
    print(f"  TkVersion: {tk.TkVersion}")
    
    # 尝试创建窗口
    try:
        root = tk.Tk()
        root.withdraw()
        print(f"✓ 可以创建Tk窗口")
        root.destroy()
    except Exception as e:
        print(f"✗ 创建Tk窗口失败: {e}")
except ImportError as e:
    print(f"✗ tkinter 导入失败: {e}")
except Exception as e:
    print(f"✗ tkinter 错误: {e}")

print()

# 检查核心模块
print("=== 检查核心模块 ===")
try:
    from bp2puerts_core import BlueprintConverter, __version__
    print(f"✓ bp2puerts_core 导入成功 (v{__version__})")
except ImportError as e:
    print(f"✗ bp2puerts_core 导入失败: {e}")
except Exception as e:
    print(f"✗ bp2puerts_core 错误: {e}")

print()

# 检查CLI模块
print("=== 检查 CLI 模块 ===")
try:
    from bp2puerts_cli import main as cli_main
    print(f"✓ bp2puerts_cli 导入成功")
except ImportError as e:
    print(f"✗ bp2puerts_cli 导入失败: {e}")
except Exception as e:
    print(f"✗ bp2puerts_cli 错误: {e}")

print()

# 检查GUI模块
print("=== 检查 GUI 模块 ===")
try:
    from bp2puerts_gui import BP2PuerTsGUI
    print(f"✓ bp2puerts_gui 导入成功")
except ImportError as e:
    print(f"✗ bp2puerts_gui 导入失败: {e}")
    import traceback
    traceback.print_exc()
except Exception as e:
    print(f"✗ bp2puerts_gui 错误: {e}")
    import traceback
    traceback.print_exc()

print()

# 尝试启动GUI
print("=== 尝试启动 GUI ===")
try:
    print("正在创建GUI实例...")
    app = BP2PuerTsGUI()
    print("✓ GUI实例创建成功")
    print("正在启动主循环...")
    app.run()
except NameError:
    print("✗ GUI类未定义 (导入失败)")
except Exception as e:
    print(f"✗ GUI启动失败: {e}")
    import traceback
    traceback.print_exc()
