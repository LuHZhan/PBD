// void UEUW_Windows::ShowContextMenu(const TArray<FEditorTabInfo>& Tabs, FVector2D ScreenPosition)
// {
// 	CloseContextMenu();
//
// 	// 广播事件（用于蓝图扩展）
// 	OnContextMenuRequested.Broadcast(Tabs, ScreenPosition);
//
// 	// 创建菜单
// 	if (ContextMenuClass && TabManager)
// 	{
// 		ActiveContextMenu = CreateWidget<UTabContextMenu>(GetWorld(), ContextMenuClass);
// 		if (ActiveContextMenu)
// 		{
// 			// 传递 GroupSubMenuClass
// 			ActiveContextMenu->GroupSubMenuClass = GroupSubMenuClass;
// 			
// 			// 🔧 修复：在编辑器中使用 Slate Window 显示 UMG Widget
// 			// 不使用 AddToViewport()，而是通过 Slate 系统显示
// 			
// 			// 初始化菜单（此时 Widget 已经有 World，因为从 GetWorld() 创建）
// 			ActiveContextMenu->InitializeMenu(TabManager, Tabs);
// 			
// 			// 创建 Slate Window 来承载 UMG Widget
// 			TSharedRef<SWindow> MenuWindow = SNew(SWindow)
// 				.Type(EWindowType::Menu)
// 				.SizingRule(ESizingRule::Autosized)
// 				.SupportsMaximize(false)
// 				.SupportsMinimize(false)
// 				.IsTopmostWindow(true)
// 				.FocusWhenFirstShown(true)
// 				.ActivationPolicy(EWindowActivationPolicy::Always)
// 				.Content()
// 				[
// 					ActiveContextMenu->TakeWidget()
// 				];
// 			
// 			// 设置窗口位置
// 			MenuWindow->MoveWindowTo(FVector2D(ScreenPosition.X, ScreenPosition.Y));
// 			
// 			// 显示窗口
// 			FSlateApplication::Get().AddWindow(MenuWindow, true);
// 			
// 			// 保存窗口引用以便后续关闭
// 			ActiveMenuWindow = MenuWindow;
// 			
// 			UE_LOG(LogTemp, Log, TEXT("[EUW_Windows] Context menu shown in Slate Window at (%.1f, %.1f)"),
// 				ScreenPosition.X, ScreenPosition.Y);
// 		}
// 	}
// }
//
// void UEUW_Windows::CloseContextMenu()
// {
// 	if (ActiveContextMenu)
// 	{
// 		ActiveContextMenu->CloseMenu();
// 		ActiveContextMenu = nullptr;
// 	}
// 	
// 	// 关闭 Slate Window
// 	if (ActiveMenuWindow.IsValid())
// 	{
// 		FSlateApplication::Get().RequestDestroyWindow(ActiveMenuWindow.Pin().ToSharedRef());
// 		ActiveMenuWindow.Reset();
// 	}
// }
