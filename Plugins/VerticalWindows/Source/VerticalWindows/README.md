# VerticalWindows TypeScript 文件说明

## 文件列表

```
TypeScript/Editor/Tab/
├── Types.ts              - 类型定义
├── EditorTabService.ts   - C++ 交互服务
├── TabItemMixin.ts       - 标签项 Mixin
├── TabGroupMixin.ts      - 分组 Mixin
├── TabManagerMixin.ts    - 主管理器 Mixin
└── index.ts              - 入口文件
```

## 蓝图路径要求

代码中硬编码的蓝图路径：

```typescript
// TabItemMixin.ts
'/VerticalWindows/Editor/WBP_TabItem.WBP_TabItem_C'

// TabGroupMixin.ts
'/VerticalWindows/Editor/WBP_TabGroup.WBP_TabGroup_C'

// TabManagerMixin.ts
'/VerticalWindows/Editor/EDU_OpenedEditor.EDU_OpenedEditor_C'
```

**如果你的蓝图路径不同，需要修改这些路径！**

---

## Widget 组件命名要求

### WBP_TabItem 必须包含的组件名称：
- `RootButton` (Button) - 整个项的点击区域
- `TitleText` (TextBlock) - 显示标签名称
- `ColorBar` (Image) - 颜色条
- `DirtyIndicator` (TextBlock) - 脏标记 (*)
- `CloseButton` (Button) - 关闭按钮
- `Background` (Border) - 背景

### WBP_TabGroup 必须包含的组件名称：
- `HeaderButton` (Button) - 分组头部点击区域
- `GroupNameText` (TextBlock) - 分组名称
- `ColorBar` (Image) - 颜色条
- `ExpandIcon` (Image) - 展开/折叠箭头
- `CountText` (TextBlock) - 计数显示
- `ItemContainer` (VerticalBox) - 子项容器

### EDU_OpenedEditor 必须包含的组件名称：
- `GroupContainer` (VerticalBox) - 分组容器
- `RefreshButton` (Button) - 刷新按钮
- `ExpandAllButton` (Button) - 全部展开按钮
- `CollapseAllButton` (Button) - 全部折叠按钮
- `SaveAllButton` (Button) - 全部保存按钮

---

## 蓝图事件连接

### EDU_OpenedEditor 蓝图：

```
Event Construct
    └── Call TS: initTabManager()

Event Destruct
    └── Call TS: cleanupTabManager()

RefreshButton.OnClicked
    └── Call TS: OnRefreshClicked()

ExpandAllButton.OnClicked
    └── Call TS: OnExpandAllClicked()

CollapseAllButton.OnClicked
    └── Call TS: OnCollapseAllClicked()

SaveAllButton.OnClicked
    └── Call TS: OnSaveAllClicked()
```

### WBP_TabItem 蓝图：

```
RootButton.OnClicked
    └── Call TS: OnItemClicked()

CloseButton.OnClicked
    └── Call TS: OnCloseClicked()
```

### WBP_TabGroup 蓝图：

```
HeaderButton.OnClicked
    └── Call TS: OnHeaderClicked()
```

---

## PuerTS Mixin 关键点

1. **`blueprint.tojs<any>(cls)`** - 泛型参数必须是类类型，用 `any` 最简单

2. **`interface Mixin extends UE.XXX`** - 让 Mixin 类能访问原类方法

3. **`blueprint.mixin(Class, Mixin)`** - 返回混入后的新类

4. **WeakMap 存储数据** - 避免内存泄漏，不阻止 GC

5. **保持 cls 引用** - 防止蓝图被 GC

---

## C++ 函数后缀

因为 `EUW_Windows` 继承自 `UEditorUtilityWidget`，PuerTS 生成的函数带 `_EditorOnly` 后缀：

```typescript
// 正确调用
this.GetAllOpenTabs_EditorOnly()
this.ActivateTab_EditorOnly(tabId)
this.CloseTab_EditorOnly(tabId)
this.SaveAllDirtyAssets_EditorOnly()
```

---

## 常见错误

### TS2344: Type 'UserWidget' does not satisfy...

```typescript
// ❌ 错误
blueprint.tojs<UE.UserWidget>(cls)

// ✅ 正确
blueprint.tojs<typeof UE.UserWidget>(cls)
// 或
blueprint.tojs<any>(cls)
```

### Cannot find file 'AssetEditorManager.h'

UE 5.3+ 使用 `UAssetEditorSubsystem`，不是 `FAssetEditorManager`：

```cpp
// 替换 include
#include "Subsystems/AssetEditorSubsystem.h"

// 替换调用
UAssetEditorSubsystem* Sub = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
Sub->GetAllEditedAssets();
```