import * as UE from 'ue'
import {argv, blueprint, $ref, $unref} from 'puerts';

console.log("📦 TS_PhyControlDog module loaded-lhz");

// ========================================
// 延迟工具函数
// ========================================
async function Delay(seconds: number): Promise<void> {
    return new Promise<void>((resolve) => {
        setTimeout(() => {
            resolve();
        }, seconds * 1000);
    });
}


// ========================================
// 1️⃣ 加载蓝图类（关键！）
// ========================================
const BlueprintPath = '/Game/PhysicsControl/BP_PhyDog.BP_PhyDog_C';
const BP_PhyDog = blueprint.tojs<typeof UE.Game.PhysicsControl.BP_PhyDog.BP_PhyDog_C>(UE.Class.Load(BlueprintPath));


// ========================================
// 2️⃣ 定义 Mixin 接口（继承蓝图类型）
// ========================================
interface PhyDogMixin extends UE.Game.PhysicsControl.BP_PhyDog.BP_PhyDog_C {}

class PhyDogMixin {
    // TS专用字段
    private physicsControlComponent: any = null;
    private isPhysicsControlInitialized: boolean = false;

    // 覆盖BeginPlay
    ReceiveBeginPlay(): void {
        console.log(`${this.GetName()} - PhyDog Mixin BeginPlay`);

        (async () => {
            await Delay(0.1);
            this.PhysicsControllerInit();
        })();
    }

    /**
     * 获取并打印当前 Actor 身上的所有组件
     */
    GetAllComponents(): UE.TArray<UE.ActorComponent> {
        const allComponents = this.K2_GetComponentsByClass(UE.ActorComponent.StaticClass());
        console.log(`\n🔍 Found ${allComponents.Num()} components on ${this.GetName()}:`);

        for (let i = 0; i < allComponents.Num(); i++) {
            const comp = allComponents.Get(i);
            const compName = comp.GetName();
            const className = comp.GetClass().GetName();
            console.log(`  [${i}] Name: ${compName} | Type: ${className}`);
        }

        return allComponents;
    }

    /**
     * 初始化PhysicsControl组件
     */
    PhysicsControllerInit(): void {
        console.log("\n🔧 Initializing PhysicsControl...");

        this.GetAllComponents();

        // 直接访问蓝图暴露的组件（如果有）
        let PhysicsController = (this as any).PhysicsControl;

        // 如果直接访问失败，通过类查找
        if (!PhysicsController) {
            try {
                const PCClass = UE.Class.Load('/Script/PhysicsControl.PhysicsControlComponent');
                if (PCClass) {
                    PhysicsController = this.GetComponentByClass(PCClass) as any;
                    if (PhysicsController) {
                        console.log("✅ PhysicsControl found via class load");
                    }
                }
            } catch (e) {
                console.log("⚠️ Class load failed:", e);
            }
        }

        if (!PhysicsController) {
            console.error("❌ PhysicsControl component not found!");
            return;
        }

        const Mesh = this.GetComponentByClass(UE.SkeletalMeshComponent.StaticClass()) as UE.SkeletalMeshComponent;
        if (!Mesh) {
            console.error("SkeletalMesh component not found!");
            return;
        }

        console.log("✅ Mesh found:", Mesh.GetName());
        this.CreatePhysicsControls(PhysicsController, Mesh);
    }

    /**
     * 创建PhysicsControl控制和修改器
     */
    private CreatePhysicsControls(PhysicsController: any, Mesh: UE.SkeletalMeshComponent): void {
        try {
            let AllWorldSpaceControls = {};
            let LimbWorldSpaceControls = UE.NewMap(UE.BuiltinName, UE.Object);
            let AllParentSpaceControls = {};
            let LimbParentSpaceControls = UE.NewMap(UE.BuiltinName, UE.Object);
            let AllBodyModifiers = {};
            let LimbBodyModifiers = UE.NewMap(UE.BuiltinName, UE.Object);
            const LimbSetupData = UE.NewArray(UE.Object);

            if (typeof PhysicsController.CreateControlsAndBodyModifiersFromLimbBones === 'function') {
                PhysicsController.CreateControlsAndBodyModifiersFromLimbBones(
                    $ref(AllWorldSpaceControls),
                    $ref(LimbWorldSpaceControls),
                    $ref(AllParentSpaceControls),
                    $ref(LimbParentSpaceControls),
                    $ref(AllBodyModifiers),
                    $ref(LimbBodyModifiers),
                    Mesh,
                    LimbSetupData,
                    undefined,
                    undefined,
                    undefined,
                    undefined,
                    ""
                );

                this.isPhysicsControlInitialized = true;
                this.physicsControlComponent = PhysicsController;
                console.log("✅ PhysicsControl initialized!");

            } else {
                console.error("❌ CreateControlsAndBodyModifiersFromLimbBones method not found!");
                for (const key in PhysicsController) {
                    if (typeof PhysicsController[key] === 'function') {
                        console.log(`  - ${key}`);
                    }
                }
            }

        } catch (error) {
            console.error("❌ Error calling CreateControlsAndBodyModifiersFromLimbBones:");
            console.error(error);
        }
    }

    TsAdd(x: number, y: number): number {
        return x + y;
    }
}

// ========================================
// 3️⃣ 应用 Mixin
// ========================================
const PhyDogWithMixin = blueprint.mixin(BP_PhyDog, PhyDogMixin);

// ========================================
// 4️⃣ 生成Actor
// ========================================
export function SpawnMixinActor(): void {
    // 多种方式获取 World
    const GameInstance = argv.getByName("GameInstance") as UE.GameInstance;
    const GameMode = argv.getByName("GameMode") as UE.GameModeBase;
    const World = argv.getByName("World") as UE.World;

    let MyWorld: UE.World | null = World;

    if (!MyWorld && GameInstance) {
        MyWorld = GameInstance.GetWorld();
    }
    if (!MyWorld && GameMode) {
        MyWorld = GameMode.GetWorld();
    }

    if (!MyWorld) {
        console.error("❌ Cannot get World! Make sure to call this after game starts.");
        return;
    }

    console.log("✅ World obtained:", MyWorld.GetName());

    try {
        const actor = UE.GameplayStatics.BeginDeferredActorSpawnFromClass(
            MyWorld,
            PhyDogWithMixin.StaticClass(),
            new UE.Transform(
                new UE.Quat(0, 0, 0, 1),
                new UE.Vector(0, 0, 200),
                new UE.Vector(1, 1, 1)
            ),
            UE.ESpawnActorCollisionHandlingMethod.AlwaysSpawn,
            undefined
        );

        if (!actor) {
            console.error("❌ Failed to spawn");
            return;
        }

        UE.GameplayStatics.FinishSpawningActor(
            actor,
            new UE.Transform(
                new UE.Quat(0, 0, 0, 1),
                new UE.Vector(0, 0, 200),
                new UE.Vector(1, 1, 1)
            )
        );

        console.log("✅ PhyDog spawned:", actor.GetName());

    } catch (error) {
        console.error("❌ Error spawning actor:", error);
    }
}