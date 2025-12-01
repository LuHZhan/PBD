#!/usr/bin/env python3
"""
bp2puerts - Blueprint to PuerTs 转换工具统一入口

根据参数自动选择CLI或GUI模式。

使用方法:
    # 无参数启动GUI
    python bp2puerts.py
    
    # 有参数使用CLI
    python bp2puerts.py input.json
    python bp2puerts.py input.json -o output.ts
    
    # 强制GUI模式
    python bp2puerts.py --gui
    
    # 强制CLI模式
    python bp2puerts.py --cli input.json

作者: Claude
版本: 1.0.0
"""

import sys
import traceback


def check_tkinter():
    """检查tkinter是否可用"""
    try:
        import tkinter as tk
        # 尝试创建一个root窗口来验证
        root = tk.Tk()
        root.withdraw()
        root.destroy()
        return True
    except Exception as e:
        print(f"[警告] tkinter不可用: {e}")
        return False


def start_gui():
    """启动GUI"""
    print("正在启动GUI...")
    try:
        from bp2puerts_gui import main as gui_main
        gui_main()
    except ImportError as e:
        print(f"[错误] GUI模块加载失败: {e}")
        print(traceback.format_exc())
        return False
    except Exception as e:
        print(f"[错误] GUI启动失败: {e}")
        print(traceback.format_exc())
        return False
    return True


def start_cli(args=None):
    """启动CLI"""
    if args is not None:
        sys.argv = [sys.argv[0]] + args
    from bp2puerts_cli import main as cli_main
    return cli_main()


def main():
    """主入口"""
    args = sys.argv[1:]
    
    # 检查是否强制指定模式
    if '--gui' in args:
        args.remove('--gui')
        sys.argv = [sys.argv[0]] + args
        if not check_tkinter():
            print("tkinter不可用，无法启动GUI")
            sys.exit(1)
        if not start_gui():
            sys.exit(1)
        return
    
    if '--cli' in args:
        args.remove('--cli')
        sys.exit(start_cli(args))
    
    # 自动选择模式
    if not args:
        # 无参数 - 尝试启动GUI，失败则显示帮助
        if check_tkinter():
            if not start_gui():
                print("\nGUI启动失败，显示CLI帮助:")
                sys.exit(start_cli(['--help']))
        else:
            print("无法启动GUI，显示CLI帮助:")
            print("使用方法: python bp2puerts.py <input.json> [-o output.ts]")
            print("详细帮助: python bp2puerts.py --help")
            sys.exit(1)
    elif len(args) == 1 and args[0] in ('-h', '--help', '-V', '--version'):
        # 帮助/版本 - 使用CLI
        sys.exit(start_cli())
    else:
        # 有参数 - 使用CLI
        sys.exit(start_cli())


if __name__ == '__main__':
    main()