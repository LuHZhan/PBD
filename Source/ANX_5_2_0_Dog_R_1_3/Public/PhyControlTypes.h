// PhyControlTypes.h
// Physics Control UI 数据结构定义
// UI只负责数据传递，实际逻辑由Actor实现

#pragma once

#include "CoreMinimal.h"
#include "PhyControlTypes.generated.h"

/**
 * 控制空间类型
 * World: 相对于世界空间控制骨骼
 * Parent: 相对于父骨骼空间控制
 */
UENUM(BlueprintType)
enum class EPhyControlSpace : uint8
{
    World   UMETA(DisplayName = "World Space"),
    Parent  UMETA(DisplayName = "Parent Space")
};

/**
 * 物理运动类型
 */
UENUM(BlueprintType)
enum class EPhyMovementType : uint8
{
    Static      UMETA(DisplayName = "Static"),
    Kinematic   UMETA(DisplayName = "Kinematic"),
    Simulated   UMETA(DisplayName = "Simulated")
};

/**
 * 单个控制空间的参数
 * Strength: 弹簧刚度，控制驱动力强度 (0-1 映射到实际值)
 * Damping: 阻尼系数，防止振荡 (0-1 映射到实际值)
 */
USTRUCT(BlueprintType)
struct FPhyControlSpaceData
{
    GENERATED_BODY()

    // 是否启用该控制空间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Control")
    bool bEnabled = false;

    // 强度 (弹簧刚度) - 值越高，驱动到目标的力越大
    // 高Strength + 低Damping = 硬限制，响应快但可能振荡
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Control", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Strength = 1.0f;

    // 阻尼 - 值越高，运动越平滑但响应越慢
    // 高Strength + 高Damping = 如同在焦油中移动，平稳但缓慢
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Control", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Damping = 1.0f;

    // 将UI值(0-1)转换为实际物理值
    float GetActualStrength() const { return Strength * 1000.0f; }  // 典型范围 0-1000
    float GetActualDamping() const { return Damping * 100.0f; }     // 典型范围 0-100
};

/**
 * Body Modifier 设置
 * 控制物理体的基本属性
 */
USTRUCT(BlueprintType)
struct FPhyBodyModifierData
{
    GENERATED_BODY()

    // P - Physics Enable: 是否启用物理模拟
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Modifier")
    bool bPhysicsEnabled = true;

    // G - Gravity: 重力开关
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Modifier")
    bool bGravityEnabled = true;

    // S - Simulate: 模拟模式 (true=Simulated, false=Kinematic)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Modifier")
    bool bSimulated = true;

    // M - Multiplier: 控制乘数 (影响整体强度)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Modifier", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float Multiplier = 1.0f;

    // B - Blend Weight: 动画与物理的混合权重
    // 0 = 纯动画, 1 = 纯物理
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Modifier", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendWeight = 1.0f;

    // 获取运动类型
    EPhyMovementType GetMovementType() const
    {
        return bSimulated ? EPhyMovementType::Simulated : EPhyMovementType::Kinematic;
    }
};

/**
 * 控制选项设置
 * R/I/W/S 按钮对应的功能
 */
USTRUCT(BlueprintType)
struct FPhyControlOptions
{
    GENERATED_BODY()

    // R - Reset: 重置到初始状态的请求标志
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Options")
    bool bResetRequested = false;

    // I - Initialize: 重新初始化控制器
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Options")
    bool bInitializeRequested = false;

    // W - World Space Override: 强制使用世界空间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Options")
    bool bWorldSpaceOverride = false;

    // S - Skeletal Animation: 是否使用骨骼动画作为目标
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control Options")
    bool bUseSkeletalAnimation = true;
};

/**
 * 完整的 Physics Control UI 数据
 * 包含所有UI状态，用于UI与Actor之间的数据传递
 */
USTRUCT(BlueprintType)
struct FPhyControlUIData
{
    GENERATED_BODY()

    // World Space 控制参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Control")
    FPhyControlSpaceData WorldSpace;

    // Parent Space 控制参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Control")
    FPhyControlSpaceData ParentSpace;

    // Body Modifier 设置 (右侧 P/G/S/M/B 按钮)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Control")
    FPhyBodyModifierData BodyModifier;

    // 控制选项 (底部 R/I/W/S 按钮)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Control")
    FPhyControlOptions Options;

    // 目标骨骼/肢体名称 (可选，用于指定控制目标)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Control")
    FName TargetLimb = NAME_None;

    // 初始化默认值
    FPhyControlUIData()
    {
        WorldSpace.bEnabled = true;
        WorldSpace.Strength = 1.0f;
        WorldSpace.Damping = 1.0f;

        ParentSpace.bEnabled = false;
        ParentSpace.Strength = 1.0f;
        ParentSpace.Damping = 1.0f;
    }
};

/**
 * UI数据变更委托
 * 当UI数据发生变化时触发
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhyControlDataChanged, const FPhyControlUIData&, NewData);

/**
 * 单项变更委托 (用于优化，避免每次都传递完整数据)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhyControlValueChanged, FName, PropertyName, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhyControlBoolChanged, FName, PropertyName, bool, NewValue);