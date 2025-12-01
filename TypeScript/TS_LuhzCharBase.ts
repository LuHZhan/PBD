/**
 * TS_LuhzCharBase
 * 使用装饰器简化组件创建
 */

import * as UE from 'ue'
import {uproperty} from 'ue'

import './ObjectExt'

class TS_LuhzCharBase extends UE.Character {

    // ============================================================
    // 组件层级结构
    // ============================================================
    //
    // Character (Self)
    // └── CapsuleComponent [原生]
    //     ├── Widget_HealthBar
    //     ├── PhysicsConstraint
    //     ├── HeightFixer
    //     ├── CheckBoxes
    //     │   ├── Crouch_Jump_check
    //     │   ├── Near_Wall_check
    //     │   ├── Interact_check
    //     │   └── Combat_check
    //     ├── SpringArm
    //     │   └── Camera
    //     ├── Head_extra_collision
    //     ├── Butt_extra_collision
    //     └── Body_Front_extra_collision

    // ============================================================
    // 第一层: 附加到 CapsuleComponent
    // ============================================================

    @uproperty.attach("CapsuleComponent")
    Widget_HealthBar: UE.WidgetComponent;

    @uproperty.attach("CapsuleComponent")
    PhysicsConstraint: UE.PhysicsConstraintComponent;

    @uproperty.attach("CapsuleComponent")
    HeightFixer: UE.StaticMeshComponent;

    @uproperty.attach("CapsuleComponent")
    CheckBoxes: UE.SceneComponent;

    @uproperty.attach("CapsuleComponent")
    SpringArm: UE.SpringArmComponent;

    @uproperty.attach("CapsuleComponent")
    Head_extra_collision: UE.CapsuleComponent;

    @uproperty.attach("CapsuleComponent")
    Butt_extra_collision: UE.CapsuleComponent;

    @uproperty.attach("CapsuleComponent")
    Body_Front_extra_collision: UE.CapsuleComponent;

    // ============================================================
    // 第二层: 附加到 CheckBoxes
    // ============================================================

    @uproperty.attach("CheckBoxes")
    Crouch_Jump_check: UE.CapsuleComponent;

    @uproperty.attach("CheckBoxes")
    Near_Wall_check: UE.CapsuleComponent;

    @uproperty.attach("CheckBoxes")
    Interact_check: UE.BoxComponent;

    @uproperty.attach("CheckBoxes")
    Combat_check: UE.BoxComponent;

    // ============================================================
    // 第三层: 附加到 SpringArm
    // ============================================================

    @uproperty.attach("SpringArm")
    Camera: UE.CameraComponent;

    // ============================================================
    // 构造函数 - 现在可以为空或设置属性默认值
    // ============================================================

    Constructor() {
        // 组件已由装饰器自动创建和附加
        // 这里可以设置组件属性
    }

    // ============================================================
    // 事件
    // ============================================================

    ReceiveBeginPlay(): void {
        UE.KismetSystemLibrary.PrintString(
            this.GetWorld(),
            "TS_LuhzCharBase Begin",
            true, true,
            new UE.LinearColor(1.0, 0.0, 0.0, 1.0),
            5.0,
            "None"
        );
    }

    ReceiveTick(DeltaSeconds: number): void {
        // 每帧更新
    }
}

export default TS_LuhzCharBase;
