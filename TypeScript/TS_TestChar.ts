/**
 * TS_TestChar
 * 使用装饰器简化组件创建
 */

import * as UE from 'ue'
import {uproperty} from 'ue'

import './ObjectExt'

class TS_TestChar extends UE.Character {

    // ============================================================
    // 组件层级结构
    // ============================================================
    //
    // Character (Self)
    // └── CapsuleComponent [原生]
    //     ├── Body_Front_extra_collision
    //     ├── Butt_extra_collision
    //     ├── Head_extra_collision
    //     ├── SpringArm
    //     │   └── Camera
    //     ├── CheckBoxes
    //     │   ├── Combat_check
    //     │   ├── Interact_check
    //     │   ├── Near_Wall_check
    //     │   └── Crouch_Jump_check
    //     ├── HeightFixer
    //     ├── PhysicsConstraint
    //     └── Widget_HealthBar

    // ============================================================
    // 第一层: 附加到 CapsuleComponent
    // ============================================================

    @uproperty.attach("CapsuleComponent")
    Body_Front_extra_collision: UE.CapsuleComponent;

    @uproperty.attach("CapsuleComponent")
    Butt_extra_collision: UE.CapsuleComponent;

    @uproperty.attach("CapsuleComponent")
    Head_extra_collision: UE.CapsuleComponent;

    @uproperty.attach("CapsuleComponent")
    SpringArm: UE.SpringArmComponent;

    @uproperty.attach("CapsuleComponent")
    CheckBoxes: UE.SceneComponent;

    @uproperty.attach("CapsuleComponent")
    HeightFixer: UE.StaticMeshComponent;

    @uproperty.attach("CapsuleComponent")
    PhysicsConstraint: UE.PhysicsConstraintComponent;

    @uproperty.attach("CapsuleComponent")
    Widget_HealthBar: UE.WidgetComponent;

    // ============================================================
    // 第二层: 附加到 SpringArm
    // ============================================================

    @uproperty.attach("SpringArm")
    Camera: UE.CameraComponent;

    // ============================================================
    // 第二层: 附加到 CheckBoxes
    // ============================================================

    @uproperty.attach("CheckBoxes")
    Combat_check: UE.BoxComponent;

    @uproperty.attach("CheckBoxes")
    Interact_check: UE.BoxComponent;

    @uproperty.attach("CheckBoxes")
    Near_Wall_check: UE.CapsuleComponent;

    @uproperty.attach("CheckBoxes")
    Crouch_Jump_check: UE.CapsuleComponent;

    // ============================================================
    // 变量
    // ============================================================

    Debug_Trace: number;
    Char_Bottom_Loc: UE.Vector;
    Look_At: UE.Vector;
    Alpha_Aim: number;
    Delta_Time: number;
    Char_Scale: number;
    Config_dis_F_for_calculating_max_height: number;
    Char_Rot: UE.Rotator;

    // ============================================================
    // 构造函数
    // ============================================================

    Constructor() {
        // 组件已由装饰器自动创建和附加
    }

    // ============================================================
    // 函数
    // ============================================================

    /**
     * Add_time - 累加时间，可重置
     */
    Add_time(/* out */ Time_to_add: number, reset: boolean): void {
        const addedTime = Time_to_add + this.Delta_Time;
        Time_to_add = reset ? 0.0 : addedTime;
    }

    /**
     * Get_Random_Index_without_repetitions - 获取不重复的随机索引
     */
    Get_Random_Index_without_repetitions(
        last_index: number,
        /* out */ Chances: number[],
        /* out */ random_index: number
    ): void {
        // 如果数组为空，返回0
        if (Chances.length === 0) {
            random_index = 0;
            return;
        }

        // 计算总概率（排除上次选中的索引）
        let totalChance = 0.0;
        for (let i = 0; i < Chances.length; i++) {
            if (i !== last_index) {
                totalChance += Chances[i];
            }
        }

        // 生成随机值
        const randomValue = UE.KismetMathLibrary.RandomFloatInRange(0.0, totalChance);

        // 查找对应的索引
        let accumulatedChance = 0.0;
        for (let i = 0; i < Chances.length; i++) {
            if (i !== last_index) {
                accumulatedChance += Chances[i];
                if (randomValue <= accumulatedChance) {
                    random_index = i;
                    return;
                }
            }
        }

        random_index = 0;
    }

    /**
     * GetMaxHight - 计算角色前方的最大高度点
     */
    GetMaxHight(/* out */ Distance: number): void {
        // 计算角色前方的点
        const forwardDir = this.Char_Rot.Vector().GetUnsafeNormal();
        const scaledDistance = this.Config_dis_F_for_calculating_max_height * this.Char_Scale;
        const forwardOffset = forwardDir * scaledDistance;
        const Point_MaxHight = this.Char_Bottom_Loc + forwardOffset;

        // 分解向量
        const [pointX, pointY, pointZ] = [Point_MaxHight.X, Point_MaxHight.Y, Point_MaxHight.Z];

        // 计算垂直射线的起点和终点
        const verticalOffset = (-999.0) * this.Char_Scale;
        const traceEndZ = pointZ + verticalOffset;

        const traceStart = new UE.Vector(pointX, pointY, pointZ);
        const traceEnd = new UE.Vector(pointX, pointY, traceEndZ);

        // 执行射线检测
        const hitResult = new UE.HitResult();
        const hit = UE.KismetSystemLibrary.LineTraceSingle(
            this,
            traceStart,
            traceEnd,
            UE.ETraceTypeQuery.TraceTypeQuery1,
            false,
            [],
            UE.EDrawDebugTrace.None,
            hitResult,
            true
        );

        Distance = hit ? hitResult.Distance : 0.0;
    }

    /**
     * Interp - 浮点数插值
     */
    Interp(
        InterpSpeed: number,
        /* out */ Current: number,
        Target: number,
        /* out */ out: number
    ): void {
        const result = UE.KismetMathLibrary.FInterpTo(Current, Target, this.Delta_Time, InterpSpeed);
        Current = result;
        out = result;
    }

    /**
     * SetLookAtTargetInFront - 设置前方注视目标
     */
    SetLookAtTargetInFront(): void {
        const actorLocation = this.K2_GetActorLocation();
        const forwardOffset = new UE.Vector(100, 0, 0); // 默认前方偏移
        this.Look_At = actorLocation + forwardOffset;
        this.Alpha_Aim = 0.0;
    }

    /**
     * Swich - 布尔切换
     */
    Swich(/* out */ A: boolean): void {
        A = !A;
    }

    /**
     * Trace_InFrontOfPlayer - 玩家前方射线检测
     */
    Trace_InFrontOfPlayer(
        /* out */ Objects: UE.EObjectTypeQuery[],
        Dis_F: number,
        Dis_Z_start: number,
        Dis_Z_end: number,
        ShowDebug_Always: boolean,
        Trase_Color: UE.LinearColor,
        DrawDebugType: number,
        /* out */ Distance: number
    ): boolean {
        // 计算射线起点和终点
        const actorLocation = this.K2_GetActorLocation();
        const forwardDir = this.GetActorForwardVector();

        const traceStart = actorLocation + forwardDir * Dis_F + new UE.Vector(0, 0, Dis_Z_start);
        const traceEnd = actorLocation + forwardDir * Dis_F + new UE.Vector(0, 0, Dis_Z_end);

        // 执行射线检测
        const hitResult = new UE.HitResult();
        const debugType = ShowDebug_Always ? UE.EDrawDebugTrace.ForDuration : UE.EDrawDebugTrace.None;

        const hit = UE.KismetSystemLibrary.LineTraceSingleForObjects(
            this,
            traceStart,
            traceEnd,
            Objects,
            false,
            [],
            debugType,
            hitResult,
            true,
            Trase_Color,
            UE.LinearColor.Green
        );

        Distance = hit ? hitResult.Distance : 0.0;
        return hit;
    }

    // ============================================================
    // 事件
    // ============================================================

    ReceiveBeginPlay(): void {
        // 初始化
    }

    ReceiveTick(DeltaSeconds: number): void {
        this.Delta_Time = DeltaSeconds;
    }
}

export default TS_TestChar;