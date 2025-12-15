// Physics/Entry.ts
import * as UE from 'ue'
import {argv} from 'puerts'
import {SpawnMixinActor} from './TS_PhyControlDog'

console.log("🔬 Physics/Entry module executing...");

// ✅ 兼容性获取参数
const GameInstance = argv.getByName("GameInstance") as UE.GameInstance;
const GameMode = argv.getByName("GameMode") as UE.GameModeBase;
const FrameManager = argv.getByName("FrameManager");
const World = argv.getByName("World") as UE.World;

// 尝试获取World
let MyWorld: UE.World | null = World;

if (!MyWorld && GameInstance) {
    console.log("Getting World from GameInstance...");
    MyWorld = GameInstance.GetWorld();
}

if (!MyWorld && GameMode) {
    console.log("Getting World from GameMode...");
    MyWorld = GameMode.GetWorld();
}

if (!MyWorld && FrameManager && (FrameManager as any).GetWorld) {
    console.log("Getting World from FrameManager...");
    MyWorld = (FrameManager as any).GetWorld();
}

// 验证并执行
if (MyWorld) {
    console.log(`✅ World obtained: ${MyWorld.GetName()}`);

    try {
        console.log("Applying Mixin...");
        SpawnMixinActor();
        console.log("✅ Grabbable object with Mixin spawned!");
    } catch (error) {
        console.error("❌ Error spawning mixin actor:", error);
    }
} else {
    console.error("❌ Failed to get World! Cannot spawn mixin actor.");
    console.error("   Available arguments:", {
        GameInstance: !!GameInstance,
        GameMode: !!GameMode,
        FrameManager: !!FrameManager,
        World: !!World
    });
}