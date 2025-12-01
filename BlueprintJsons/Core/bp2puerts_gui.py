#!/usr/bin/env python3
"""
bp2puerts_gui.py - Blueprint to PuerTs 图形界面

基于tkinter实现，无需额外依赖。

功能:
- 文件选择/拖拽
- 实时预览
- 配置选项
- 批量转换
- 转换历史

使用方法:
    python bp2puerts_gui.py

作者: Claude
版本: 1.0.0
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext
from pathlib import Path
import threading
import queue
import json
from typing import Optional, List

# 导入核心模块
try:
    from bp2puerts_core import (
        BlueprintConverter,
        ConvertConfig,
        ConvertResult,
        ParsedBlueprint,
        __version__,
    )
except ImportError:
    import sys
    import os
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from bp2puerts_core import (
        BlueprintConverter,
        ConvertConfig,
        ConvertResult,
        ParsedBlueprint,
        __version__,
    )


# ============================================
# 主应用类
# ============================================

class BP2PuerTsGUI:
    """Blueprint to PuerTs 图形界面"""
    
    def __init__(self):
        self.root = tk.Tk()
        self.root.title(f"Blueprint to PuerTs Converter v{__version__}")
        self.root.geometry("1200x800")
        self.root.minsize(900, 600)
        
        # 状态
        self.current_file: Optional[str] = None
        self.current_result: Optional[ConvertResult] = None
        self.config = ConvertConfig()
        self.task_queue = queue.Queue()
        
        # 配置变量
        self.var_include_comments = tk.BooleanVar(value=True)
        self.var_include_bytecode = tk.BooleanVar(value=True)
        self.var_generate_helpers = tk.BooleanVar(value=True)
        self.var_group_props = tk.BooleanVar(value=True)
        self.var_group_funcs = tk.BooleanVar(value=True)
        self.var_use_async = tk.BooleanVar(value=True)
        self.var_max_props = tk.StringVar(value="100")
        self.var_max_funcs = tk.StringVar(value="50")
        
        # 创建界面
        self._create_menu()
        self._create_toolbar()
        self._create_main_content()
        self._create_statusbar()
        
        # 绑定事件
        self._bind_events()
        
        # 启动后台任务处理
        self._process_queue()
    
    def run(self):
        """运行应用"""
        self.root.mainloop()
    
    # ========== 界面创建 ==========
    
    def _create_menu(self):
        """创建菜单栏"""
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)
        
        # 文件菜单
        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="文件", menu=file_menu)
        file_menu.add_command(label="打开 JSON...", command=self._open_file, accelerator="Ctrl+O")
        file_menu.add_command(label="保存 TypeScript...", command=self._save_file, accelerator="Ctrl+S")
        file_menu.add_separator()
        file_menu.add_command(label="批量转换...", command=self._batch_convert)
        file_menu.add_separator()
        file_menu.add_command(label="退出", command=self.root.quit, accelerator="Ctrl+Q")
        
        # 编辑菜单
        edit_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="编辑", menu=edit_menu)
        edit_menu.add_command(label="复制代码", command=self._copy_code, accelerator="Ctrl+C")
        edit_menu.add_separator()
        edit_menu.add_command(label="配置...", command=self._show_config_dialog)
        
        # 帮助菜单
        help_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="帮助", menu=help_menu)
        help_menu.add_command(label="使用说明", command=self._show_help)
        help_menu.add_command(label="关于", command=self._show_about)
    
    def _create_toolbar(self):
        """创建工具栏"""
        toolbar = ttk.Frame(self.root)
        toolbar.pack(fill=tk.X, padx=5, pady=5)
        
        # 打开按钮
        self.btn_open = ttk.Button(toolbar, text="📂 打开", command=self._open_file)
        self.btn_open.pack(side=tk.LEFT, padx=2)
        
        # 转换按钮
        self.btn_convert = ttk.Button(toolbar, text="🔄 转换", command=self._convert, state=tk.DISABLED)
        self.btn_convert.pack(side=tk.LEFT, padx=2)
        
        # 保存按钮
        self.btn_save = ttk.Button(toolbar, text="💾 保存", command=self._save_file, state=tk.DISABLED)
        self.btn_save.pack(side=tk.LEFT, padx=2)
        
        ttk.Separator(toolbar, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=10)
        
        # 配置按钮
        self.btn_config = ttk.Button(toolbar, text="⚙️ 配置", command=self._show_config_dialog)
        self.btn_config.pack(side=tk.LEFT, padx=2)
        
        # 文件路径显示
        self.lbl_file = ttk.Label(toolbar, text="未选择文件", foreground="gray")
        self.lbl_file.pack(side=tk.RIGHT, padx=10)
    
    def _create_main_content(self):
        """创建主内容区"""
        # 使用PanedWindow实现可调整大小的分割
        self.paned = ttk.PanedWindow(self.root, orient=tk.HORIZONTAL)
        self.paned.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # 左侧面板 - 蓝图信息
        left_frame = ttk.Frame(self.paned)
        self.paned.add(left_frame, weight=1)
        
        # 信息标签页
        self.info_notebook = ttk.Notebook(left_frame)
        self.info_notebook.pack(fill=tk.BOTH, expand=True)
        
        # 概览标签页
        overview_frame = ttk.Frame(self.info_notebook)
        self.info_notebook.add(overview_frame, text="概览")
        
        self.overview_text = scrolledtext.ScrolledText(
            overview_frame, wrap=tk.WORD, width=40, height=20
        )
        self.overview_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        self.overview_text.config(state=tk.DISABLED)
        
        # 属性标签页
        props_frame = ttk.Frame(self.info_notebook)
        self.info_notebook.add(props_frame, text="属性")
        
        self.props_tree = ttk.Treeview(
            props_frame, columns=("type", "category"), show="headings"
        )
        self.props_tree.heading("type", text="类型")
        self.props_tree.heading("category", text="分类")
        self.props_tree.column("type", width=150)
        self.props_tree.column("category", width=80)
        
        props_scroll = ttk.Scrollbar(props_frame, orient=tk.VERTICAL, command=self.props_tree.yview)
        self.props_tree.configure(yscrollcommand=props_scroll.set)
        
        self.props_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        props_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        
        # 函数标签页
        funcs_frame = ttk.Frame(self.info_notebook)
        self.info_notebook.add(funcs_frame, text="函数")
        
        self.funcs_tree = ttk.Treeview(
            funcs_frame, columns=("params", "return", "category"), show="headings"
        )
        self.funcs_tree.heading("params", text="参数")
        self.funcs_tree.heading("return", text="返回值")
        self.funcs_tree.heading("category", text="分类")
        self.funcs_tree.column("params", width=150)
        self.funcs_tree.column("return", width=80)
        self.funcs_tree.column("category", width=80)
        
        funcs_scroll = ttk.Scrollbar(funcs_frame, orient=tk.VERTICAL, command=self.funcs_tree.yview)
        self.funcs_tree.configure(yscrollcommand=funcs_scroll.set)
        
        self.funcs_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        funcs_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        
        # 右侧面板 - 代码预览
        right_frame = ttk.Frame(self.paned)
        self.paned.add(right_frame, weight=2)
        
        # 代码标题
        code_header = ttk.Frame(right_frame)
        code_header.pack(fill=tk.X)
        
        ttk.Label(code_header, text="TypeScript 代码预览", font=("", 10, "bold")).pack(side=tk.LEFT, padx=5)
        
        self.lbl_lines = ttk.Label(code_header, text="", foreground="gray")
        self.lbl_lines.pack(side=tk.RIGHT, padx=5)
        
        # 代码文本框
        self.code_text = scrolledtext.ScrolledText(
            right_frame, wrap=tk.NONE, font=("Consolas", 10)
        )
        self.code_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # 水平滚动条
        code_xscroll = ttk.Scrollbar(right_frame, orient=tk.HORIZONTAL, command=self.code_text.xview)
        self.code_text.configure(xscrollcommand=code_xscroll.set)
        code_xscroll.pack(fill=tk.X)
        
        # 语法高亮标签
        self.code_text.tag_configure("keyword", foreground="#0000FF")
        self.code_text.tag_configure("string", foreground="#008000")
        self.code_text.tag_configure("comment", foreground="#808080")
        self.code_text.tag_configure("type", foreground="#267F99")
    
    def _create_statusbar(self):
        """创建状态栏"""
        self.statusbar = ttk.Frame(self.root)
        self.statusbar.pack(fill=tk.X, side=tk.BOTTOM)
        
        self.status_label = ttk.Label(self.statusbar, text="就绪")
        self.status_label.pack(side=tk.LEFT, padx=10)
        
        self.progress = ttk.Progressbar(self.statusbar, mode='indeterminate', length=100)
        self.progress.pack(side=tk.RIGHT, padx=10)
    
    def _bind_events(self):
        """绑定事件"""
        self.root.bind('<Control-o>', lambda e: self._open_file())
        self.root.bind('<Control-s>', lambda e: self._save_file())
        self.root.bind('<Control-q>', lambda e: self.root.quit())
        self.root.bind('<Control-c>', lambda e: self._copy_code())
        
        # 拖拽支持 (仅Windows)
        try:
            self.root.drop_target_register('DND_Files')
            self.root.dnd_bind('<<Drop>>', self._on_drop)
        except:
            pass
    
    # ========== 事件处理 ==========
    
    def _open_file(self):
        """打开文件"""
        filepath = filedialog.askopenfilename(
            title="选择Blueprint JSON文件",
            filetypes=[
                ("JSON files", "*.json"),
                ("All files", "*.*"),
            ]
        )
        
        if filepath:
            self._load_file(filepath)
    
    def _load_file(self, filepath: str):
        """加载文件"""
        self.current_file = filepath
        self.lbl_file.config(text=Path(filepath).name)
        self._set_status(f"正在加载: {filepath}")
        
        # 在后台线程中加载和解析
        def task():
            try:
                converter = BlueprintConverter(filepath, self._build_config())
                result = converter.convert()
                self.task_queue.put(('load_complete', result))
            except Exception as e:
                self.task_queue.put(('error', str(e)))
        
        self.progress.start()
        threading.Thread(target=task, daemon=True).start()
    
    def _convert(self):
        """执行转换"""
        if not self.current_file:
            return
        
        self._set_status("正在转换...")
        
        def task():
            try:
                converter = BlueprintConverter(self.current_file, self._build_config())
                result = converter.convert()
                self.task_queue.put(('convert_complete', result))
            except Exception as e:
                self.task_queue.put(('error', str(e)))
        
        self.progress.start()
        threading.Thread(target=task, daemon=True).start()
    
    def _save_file(self):
        """保存文件"""
        if not self.current_result or not self.current_result.success:
            messagebox.showwarning("警告", "没有可保存的内容")
            return
        
        default_name = Path(self.current_file).stem + ".ts" if self.current_file else "output.ts"
        
        filepath = filedialog.asksaveasfilename(
            title="保存TypeScript文件",
            defaultextension=".ts",
            initialfile=default_name,
            filetypes=[
                ("TypeScript files", "*.ts"),
                ("All files", "*.*"),
            ]
        )
        
        if filepath:
            Path(filepath).write_text(self.current_result.code, encoding='utf-8')
            self._set_status(f"已保存: {filepath}")
            messagebox.showinfo("成功", f"文件已保存到:\n{filepath}")
    
    def _batch_convert(self):
        """批量转换"""
        filepaths = filedialog.askopenfilenames(
            title="选择多个Blueprint JSON文件",
            filetypes=[
                ("JSON files", "*.json"),
                ("All files", "*.*"),
            ]
        )
        
        if not filepaths:
            return
        
        output_dir = filedialog.askdirectory(title="选择输出目录")
        
        if not output_dir:
            return
        
        self._set_status(f"正在批量转换 {len(filepaths)} 个文件...")
        
        def task():
            success = 0
            failed = 0
            config = self._build_config()
            
            for filepath in filepaths:
                try:
                    converter = BlueprintConverter(filepath, config)
                    result = converter.convert()
                    
                    if result.success:
                        output_path = Path(output_dir) / (Path(filepath).stem + ".ts")
                        output_path.write_text(result.code, encoding='utf-8')
                        success += 1
                    else:
                        failed += 1
                except:
                    failed += 1
            
            self.task_queue.put(('batch_complete', (success, failed)))
        
        self.progress.start()
        threading.Thread(target=task, daemon=True).start()
    
    def _copy_code(self):
        """复制代码到剪贴板"""
        if self.current_result and self.current_result.success:
            self.root.clipboard_clear()
            self.root.clipboard_append(self.current_result.code)
            self._set_status("代码已复制到剪贴板")
    
    def _on_drop(self, event):
        """处理文件拖拽"""
        filepath = event.data
        if filepath.startswith('{') and filepath.endswith('}'):
            filepath = filepath[1:-1]
        
        if filepath.lower().endswith('.json'):
            self._load_file(filepath)
    
    # ========== 配置对话框 ==========
    
    def _show_config_dialog(self):
        """显示配置对话框"""
        dialog = tk.Toplevel(self.root)
        dialog.title("转换配置")
        dialog.geometry("400x450")
        dialog.transient(self.root)
        dialog.grab_set()
        
        # 主框架
        main_frame = ttk.Frame(dialog, padding=10)
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # 输出选项
        output_frame = ttk.LabelFrame(main_frame, text="输出选项", padding=10)
        output_frame.pack(fill=tk.X, pady=5)
        
        ttk.Checkbutton(output_frame, text="包含注释", variable=self.var_include_comments).pack(anchor=tk.W)
        ttk.Checkbutton(output_frame, text="包含字节码提示", variable=self.var_include_bytecode).pack(anchor=tk.W)
        ttk.Checkbutton(output_frame, text="生成辅助函数", variable=self.var_generate_helpers).pack(anchor=tk.W)
        ttk.Checkbutton(output_frame, text="使用async/await处理Delay", variable=self.var_use_async).pack(anchor=tk.W)
        
        # 分组选项
        group_frame = ttk.LabelFrame(main_frame, text="分组选项", padding=10)
        group_frame.pack(fill=tk.X, pady=5)
        
        ttk.Checkbutton(group_frame, text="按类别分组属性", variable=self.var_group_props).pack(anchor=tk.W)
        ttk.Checkbutton(group_frame, text="按类别分组函数", variable=self.var_group_funcs).pack(anchor=tk.W)
        
        # 限制选项
        limit_frame = ttk.LabelFrame(main_frame, text="数量限制", padding=10)
        limit_frame.pack(fill=tk.X, pady=5)
        
        ttk.Label(limit_frame, text="最大属性数 (0=无限):").pack(anchor=tk.W)
        ttk.Entry(limit_frame, textvariable=self.var_max_props, width=10).pack(anchor=tk.W, pady=2)
        
        ttk.Label(limit_frame, text="最大函数数 (0=无限):").pack(anchor=tk.W)
        ttk.Entry(limit_frame, textvariable=self.var_max_funcs, width=10).pack(anchor=tk.W, pady=2)
        
        # 按钮
        btn_frame = ttk.Frame(main_frame)
        btn_frame.pack(fill=tk.X, pady=10)
        
        ttk.Button(btn_frame, text="应用并重新转换", command=lambda: self._apply_config(dialog)).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="取消", command=dialog.destroy).pack(side=tk.RIGHT, padx=5)
    
    def _apply_config(self, dialog):
        """应用配置"""
        dialog.destroy()
        if self.current_file:
            self._convert()
    
    def _build_config(self) -> ConvertConfig:
        """从GUI变量构建配置"""
        config = ConvertConfig()
        config.include_comments = self.var_include_comments.get()
        config.include_bytecode_hints = self.var_include_bytecode.get()
        config.generate_helper_functions = self.var_generate_helpers.get()
        config.group_properties = self.var_group_props.get()
        config.group_functions = self.var_group_funcs.get()
        config.use_async_delay = self.var_use_async.get()
        
        try:
            config.max_properties = int(self.var_max_props.get())
        except:
            config.max_properties = 100
        
        try:
            config.max_functions = int(self.var_max_funcs.get())
        except:
            config.max_functions = 50
        
        return config
    
    # ========== 显示更新 ==========
    
    def _update_display(self, result: ConvertResult):
        """更新显示"""
        self.current_result = result
        
        if not result.success:
            self._set_status("转换失败")
            messagebox.showerror("错误", "\n".join(result.errors))
            return
        
        bp = result.blueprint
        
        # 更新概览
        self.overview_text.config(state=tk.NORMAL)
        self.overview_text.delete(1.0, tk.END)
        
        overview = f"""类名: {bp.class_name}
原始名: {bp.original_name}
父类: {bp.parent_class}
路径: {bp.folder_name}

统计:
  属性: {result.stats.get('properties', 0)} 个
  组件: {result.stats.get('components', 0)} 个
  函数: {result.stats.get('functions', 0)} 个
  事件: {result.stats.get('events', 0)} 个
  代码: {result.stats.get('lines', 0)} 行
"""
        self.overview_text.insert(tk.END, overview)
        self.overview_text.config(state=tk.DISABLED)
        
        # 更新属性列表
        self.props_tree.delete(*self.props_tree.get_children())
        for prop in bp.properties:
            self.props_tree.insert("", tk.END, text=prop.name, values=(prop.type, prop.category.name))
        
        # 更新函数列表
        self.funcs_tree.delete(*self.funcs_tree.get_children())
        for func in bp.functions:
            params = ", ".join(p.name for p in func.params)
            self.funcs_tree.insert("", tk.END, text=func.name, values=(params, func.return_type, func.category.name))
        
        # 更新代码
        self.code_text.delete(1.0, tk.END)
        self.code_text.insert(tk.END, result.code)
        self._highlight_code()
        
        self.lbl_lines.config(text=f"{result.stats.get('lines', 0)} 行")
        
        # 启用按钮
        self.btn_convert.config(state=tk.NORMAL)
        self.btn_save.config(state=tk.NORMAL)
        
        self._set_status(f"转换完成: {bp.class_name}")
    
    def _highlight_code(self):
        """简单的语法高亮"""
        content = self.code_text.get(1.0, tk.END)
        
        # 关键字
        keywords = ['import', 'from', 'class', 'extends', 'function', 'return', 
                   'if', 'else', 'for', 'while', 'const', 'let', 'var', 'new',
                   'async', 'await', 'export', 'default', 'this', 'true', 'false']
        
        for keyword in keywords:
            start = 1.0
            while True:
                pos = self.code_text.search(r'\b' + keyword + r'\b', start, tk.END, regexp=True)
                if not pos:
                    break
                end = f"{pos}+{len(keyword)}c"
                self.code_text.tag_add("keyword", pos, end)
                start = end
        
        # 注释
        start = 1.0
        while True:
            pos = self.code_text.search("//", start, tk.END)
            if not pos:
                break
            line_end = self.code_text.index(f"{pos} lineend")
            self.code_text.tag_add("comment", pos, line_end)
            start = f"{pos}+1l"
        
        # 字符串
        start = 1.0
        while True:
            pos = self.code_text.search(r"'[^']*'", start, tk.END, regexp=True)
            if not pos:
                break
            match_end = self.code_text.search("'", f"{pos}+1c", tk.END)
            if match_end:
                self.code_text.tag_add("string", pos, f"{match_end}+1c")
                start = f"{match_end}+1c"
            else:
                break
    
    def _set_status(self, text: str):
        """设置状态栏文本"""
        self.status_label.config(text=text)
    
    # ========== 帮助对话框 ==========
    
    def _show_help(self):
        """显示帮助"""
        help_text = """Blueprint to PuerTs Converter 使用说明

1. 打开文件
   - 点击"打开"按钮选择JSON文件
   - 或直接拖拽文件到窗口

2. 查看信息
   - 左侧面板显示蓝图的属性和函数信息
   - 可以在不同标签页间切换

3. 转换配置
   - 点击"配置"按钮调整转换选项
   - 包括注释、分组、数量限制等

4. 保存结果
   - 点击"保存"按钮保存TypeScript代码
   - 也可以使用"复制代码"功能

5. 批量转换
   - 使用"文件-批量转换"功能
   - 选择多个JSON文件和输出目录

快捷键:
  Ctrl+O  打开文件
  Ctrl+S  保存文件
  Ctrl+C  复制代码
  Ctrl+Q  退出
"""
        messagebox.showinfo("使用说明", help_text)
    
    def _show_about(self):
        """显示关于"""
        about_text = f"""Blueprint to PuerTs Converter
版本: {__version__}

将UAssetGUI导出的蓝图JSON转换为
PuerTs TypeScript代码。

作者: Claude
"""
        messagebox.showinfo("关于", about_text)
    
    # ========== 后台任务处理 ==========
    
    def _process_queue(self):
        """处理后台任务队列"""
        try:
            while True:
                msg_type, data = self.task_queue.get_nowait()
                
                self.progress.stop()
                
                if msg_type == 'load_complete':
                    self._update_display(data)
                
                elif msg_type == 'convert_complete':
                    self._update_display(data)
                
                elif msg_type == 'batch_complete':
                    success, failed = data
                    self._set_status(f"批量转换完成: 成功 {success}, 失败 {failed}")
                    messagebox.showinfo("完成", f"批量转换完成\n成功: {success}\n失败: {failed}")
                
                elif msg_type == 'error':
                    self._set_status(f"错误: {data}")
                    messagebox.showerror("错误", data)
        
        except queue.Empty:
            pass
        
        self.root.after(100, self._process_queue)


# ============================================
# 主入口
# ============================================

def main():
    """主入口"""
    app = BP2PuerTsGUI()
    app.run()


if __name__ == '__main__':
    main()