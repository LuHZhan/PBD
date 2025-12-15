import * as UE from 'ue'
import {argv} from 'puerts';

console.log("=== LearPuerTsQuickStart Started ===");

// ✅ 兼容性获取参数 - 尝试多种可能的参数名
let GameMode = argv.getByName("GameMode") as UE.GameModeBase;
let GameInstance = argv.getByName("GameInstance") as UE.GameInstance;
let FrameManager = argv.getByName("FrameManager");
let World = argv.getByName("World") as UE.World;

// 打印接收到的参数
console.log("Received arguments:");
console.log("  GameMode:", GameMode?.GetName());
console.log("  GameInstance:", GameInstance?.GetName());
console.log("  FrameManager:", FrameManager?.GetName());
console.log("  World:", World?.GetName());

// ✅ 尝试获取World（多种方式）
let MyWorld: UE.World | null = World;

if (!MyWorld && GameMode) {
    console.log("Getting World from GameMode...");
    MyWorld = GameMode.GetWorld();
}

if (!MyWorld && GameInstance) {
    console.log("Getting World from GameInstance...");
    MyWorld = GameInstance.GetWorld();
}

if (!MyWorld && FrameManager && (FrameManager as any).GetWorld) {
    console.log("Getting World from FrameManager...");
    MyWorld = (FrameManager as any).GetWorld();
}

// 验证World
if (MyWorld) {
    console.log(`✅ World obtained: ${MyWorld.GetName()}`);

    UE.KismetSystemLibrary.PrintString(
        MyWorld,
        "脚本中获取到World和GameMode",
        true,
        true,
        new UE.LinearColor(1.0, 0.0, 0.0, 1.0),
        5.0,
        "None"
    );

    // ✅ 动态加载Physics模块
    console.log("\n📦 Loading Physics module...");

    async function loadModules() {
        try {
            // 动态导入
            await import('./Physics/Entry');
            console.log("✅ Physics/Entry loaded successfully");
        } catch (error) {
            console.error("❌ Error loading Physics/Entry:", error);
        }
    }

    loadModules();

} else {
    console.error("❌ Failed to get World from any source!");
    console.error("   Available arguments:", {
        GameMode: !!GameMode,
        GameInstance: !!GameInstance,
        FrameManager: !!FrameManager,
        World: !!World
    });
}

console.log("=== LearPuerTsQuickStart Finished ===");