import * as UE from 'ue'
import {argv, blueprint, $ref, $unref} from 'puerts';

console.log("📦 TS_PhyControlDog module loaded");

// ========================================
// 1️⃣ 定义中转类（声明所有组件）
// ========================================
interface PhyDogMixin extends UE.Actor {
    // 基础组件
    CapsuleComponent?: UE.CapsuleComponent;
    Mesh?: UE.SkeletalMeshComponent;
    ArrowComponent?: UE.ArrowComponent;

    // PhysicsControl组件
    PhysicsControl?: UE.SceneComponent;
};

class PhyDogMixin implements PhyDogMixin {
}

Object.setPrototypeOf(PhyDogMixin.prototype, UE.Actor.prototype);

// ========================================
// 2️⃣ 定义Mixin实现类
// ========================================
class PhyDogMixinImpl extends PhyDogMixin {
    // TS专用字段
    private tsOnlyField: number = 0;
    private physicsControlComponent: any = null;
    private isPhysicsControlInitialized: boolean = false;

    // 覆盖BeginPlay
    ReceiveBeginPlay(): void {
        console.log(`${this.GetName()} - PhyDog Mixin BeginPlay`);

        // 初始化PhysicsControl
        this.PhysicsControllerInit();
    }

    /**
     * 初始化PhysicsControl组件
     */
    PhysicsControllerInit(): void {
        console.log("\n🔧 Initializing PhysicsControl...");

        // 1. 获取PhysicsControl组件
        // 方式1: 直接访问
        let PhysicsController: any = this.PhysicsControl;

        // 方式2: 通过类型查找
        if (!PhysicsController) {
            try {
                const PCClass = UE.Class.Load('/Script/PhysicsControl.PhysicsControlComponent');
                if (PCClass) {
                    PhysicsController = this.GetComponentByClass(PCClass) as any;
                    console.log("✅ PhysicsControl found via class load");
                }
            } catch (e) {
                console.log("⚠️ Class load failed, searching by name...");
            }
        }

        if (!PhysicsController) {
            console.error("PhysicsControl component not found!");
            this.DebugPrintAllComponents();
            return;
        }

        // 2. 获取SkeletalMesh组件
        const Mesh = this.GetComponentByClass(UE.SkeletalMeshComponent.StaticClass()) as UE.SkeletalMeshComponent;

        if (!Mesh) {
            console.error("SkeletalMesh component not found!");
            return;
        }

        console.log("✅ Mesh found:", Mesh.GetName());

        // 3. 调用CreateControlsAndBodyModifiersFromLimbBones
        this.CreatePhysicsControls(PhysicsController, Mesh);
    }

    /**
     * 创建PhysicsControl控制和修改器
     */
    private CreatePhysicsControls(PhysicsController: any, Mesh: UE.SkeletalMeshComponent): void {
        console.log("\n⚡ Creating physics controls and body modifiers...");

        try {
            // ✅ 准备输出参数（使用$ref包装）
            let AllWorldSpaceControls = {};
            let LimbWorldSpaceControls = UE.NewMap(UE.BuiltinName, UE.Object);
            let AllParentSpaceControls = {};
            let LimbParentSpaceControls = UE.NewMap(UE.BuiltinName, UE.Object);
            let AllBodyModifiers = {};
            let LimbBodyModifiers = UE.NewMap(UE.BuiltinName, UE.Object);

            // ✅ 准备输入参数
            // LimbSetupData - 空数组或根据需求填充
            const LimbSetupData = UE.NewArray(UE.Object);  // 暂时使用Object，实际应为PhysicsControlLimbSetupData

            // 如果需要配置Limb数据：
            // const limbData = {}; // 或者 new UE.PhysicsControlLimbSetupData()
            // limbData.LimbName = "LeftArm";
            // LimbSetupData.Add(limbData);

            console.log("Calling CreateControlsAndBodyModifiersFromLimbBones...");
            console.log(`  SkeletalMesh: ${Mesh.GetName()}`);
            console.log(`  LimbSetupData count: ${LimbSetupData.Num()}`);

            // ✅ 调用函数
            if (typeof PhysicsController.CreateControlsAndBodyModifiersFromLimbBones === 'function') {
                PhysicsController.CreateControlsAndBodyModifiersFromLimbBones(
                    $ref(AllWorldSpaceControls),      // 引用参数
                    $ref(LimbWorldSpaceControls),
                    $ref(AllParentSpaceControls),
                    $ref(LimbParentSpaceControls),
                    $ref(AllBodyModifiers),
                    $ref(LimbBodyModifiers),
                    Mesh,                             // ✅ 输入：SkeletalMesh
                    LimbSetupData,                    // ✅ 输入：LimbSetupData
                    undefined,                        // 使用默认值
                    undefined,
                    undefined,
                    undefined,
                    ""
                );

                console.log("✅ CreateControlsAndBodyModifiersFromLimbBones called successfully");

                // 获取输出结果（如果需要）
                console.log("Output - AllWorldSpaceControls:", $unref(AllWorldSpaceControls));
                console.log("Output - AllParentSpaceControls:", $unref(AllParentSpaceControls));

                this.isPhysicsControlInitialized = true;
                this.physicsControlComponent = PhysicsController;

            } else {
                console.error("❌ CreateControlsAndBodyModifiersFromLimbBones method not found!");
                console.log("Available methods:");

                // 打印所有可用方法
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

    /**
     * 调试：打印所有组件
     */
    private DebugPrintAllComponents(): void {
        console.log("\n🔍 === All Components ===");

        const AllComponents = this.GetComponentsByClass(UE.ActorComponent.StaticClass());

        if (AllComponents) {
            console.log(`Total: ${AllComponents.Num()} components\n`);

            for (let i = 0; i < AllComponents.Num(); i++) {
                const Comp = AllComponents.Get(i);
                const ClassName = Comp.GetClass().GetName();
                const CompName = Comp.GetName();

                console.log(`[${i}] ${ClassName}`);
                console.log(`    Name: ${CompName}`);
                console.log("");
            }
        }

        console.log("=".repeat(50));
    }

    // 覆盖蓝图函数
    Log(msg: string): void {
        console.log(this.GetName(), msg);

        if (this.isPhysicsControlInitialized) {
            console.log("  PhysicsControl: ✅ Initialized");
        } else {
            console.log("  PhysicsControl: ❌ Not initialized");
        }
    }

    // 新增TS方法
    TsAdd(x: number, y: number): number {
        return x + y;
    }
}

// ========================================
// 3️⃣ 应用Mixin函数
// ========================================
export function ApplyMixin(): UE.Class | null {
    const BlueprintPath = '/Game/PhysicsControl/BP_PhyDog.BP_PhyDog_C';
    console.log(`\n📂 Loading blueprint: ${BlueprintPath}`);

    const ucls = UE.Class.Load(BlueprintPath) as UE.BlueprintGeneratedClass;

    if (!ucls) {
        console.error(`❌ Failed to load blueprint: ${BlueprintPath}`);
        return null;
    }

    console.log("✅ Blueprint loaded");

    try {
        const BlueprintClass = blueprint.tojs(ucls);
        const MixinedClass = blueprint.mixin(BlueprintClass, PhyDogMixinImpl, {
            inherit: true,
            objectTakeByNative: true
        });

        console.log("✅ Mixin applied");
        return MixinedClass.StaticClass();
    } catch (error) {
        console.error("❌ Error applying mixin:", error);
        return null;
    }
}

// ========================================
// 4️⃣ 生成Actor
// ========================================
export function SpawnMixinActor(): void {
    console.log("\n🎯 SpawnMixinActor called");

    const MixinClass = ApplyMixin();
    if (!MixinClass) return;

    // 获取World
    const GameInstance = argv.getByName("GameInstance") as UE.GameInstance;
    const GameMode = argv.getByName("GameMode") as UE.GameModeBase;
    const FrameManager = argv.getByName("FrameManager");
    const World = argv.getByName("World") as UE.World;

    let MyWorld: UE.World | null = World;

    if (!MyWorld && GameInstance) {
        MyWorld = GameInstance.GetWorld();
    }
    if (!MyWorld && GameMode) {
        MyWorld = GameMode.GetWorld();
    }
    if (!MyWorld && FrameManager && (FrameManager as any).GetWorld) {
        MyWorld = (FrameManager as any).GetWorld();
    }

    if (!MyWorld) {
        console.error("❌ Cannot get World!");
        return;
    }

    console.log(`✅ World: ${MyWorld.GetName()}`);

    try {
        console.log("Spawning PhyDog actor...");

        const actor = UE.GameplayStatics.BeginDeferredActorSpawnFromClass(
            MyWorld,
            MixinClass,
            new UE.Transform(
                new UE.Quat(0, 0, 0, 1),
                new UE.Vector(0, 0, 200),  // 在空中生成
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

        console.log(`✅ PhyDog spawned: ${actor.GetName()}`);

        // 调用Mixin方法
        const phyDog = actor as unknown as PhyDogMixinImpl;
        phyDog.Log("PhyDog with PhysicsControl initialized!");

        console.log("\n✅ PhyDog Mixin完全初始化成功！");

    } catch (error) {
        console.error("❌ Error spawning actor:", error);
    }
}
