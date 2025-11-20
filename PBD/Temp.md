看这段代码，`Bead`类使用的是**Position Based Dynamics (PBD)**的Verlet积分方案来模拟约束动力学。让我详细解析其物理模拟方式：

## 核心算法：PBD with Verlet Integration

### 1. 时间积分方案 - `startStep()`

```javascript
startStep(dt, gravity) {
    this.vel.add(gravity, dt);        // v = v + g·Δt
    this.prevPos.set(this.pos);       // 保存上一帧位置
    this.pos.add(this.vel, dt);       // x_predicted = x + v·Δt
}
```

这是**半隐式Euler积分**的预测步骤：
- 先更新速度：施加外力（重力）
- 保存当前位置用于后续速度重构
- 预测新位置（此时可能违反约束）

### 2. 约束投影 - `keepOnWire()`

```javascript
keepOnWire(center, radius) {
    var dir = new Vector2();
    dir.subtractVectors(this.pos, center);  // 从圆心到珠子的向量
    var len = dir.length();                 // 当前距离
    if (len == 0.0) return;
    
    dir.scale(1.0 / len);                   // 归一化方向
    var lambda = physicsScene.wireRadius - len;  // 约束违反量
    this.pos.add(dir, lambda);              // 位置修正
    return lambda;
}
```

这是PBD的**约束求解核心**：
- **约束函数**：`C(x) = |x - center| - radius = 0`
- **约束违反量**：`λ = radius - |x - center|`
- **梯度方向**：`∇C = (x - center)/|x - center|`
- **位置修正**：`Δx = λ·∇C`

关键点：
- 这里使用**直接投影**而不是迭代求解
- 对于圆形约束，投影是精确的（一步到位）
- `lambda`不仅是位置修正量，也可用于估算约束力

### 3. 速度更新 - `endStep()`

```javascript
endStep(dt) {
    this.vel.subtractVectors(this.pos, this.prevPos);  // Δx
    this.vel.scale(1.0 / dt);                          // v = Δx/Δt
}
```

这是**Verlet积分的速度重构**：
- 基于位置差分计算速度：`v = (x_new - x_old)/Δt`
- 隐式包含了约束力的影响
- 保证动量守恒

## 物理原理深度解析

### 为什么这种方法有效？

1. **隐式约束力**
   ```
   约束力 ≈ m·λ/Δt²
   ```
   虽然没有显式计算力，但位置修正`λ`隐含了约束力信息

2. **能量守恒特性**
   Verlet积分对于振荡系统有较好的能量守恒性：
   - 位置的二阶精度：`O(Δt²)`
   - 速度的一阶精度：`O(Δt)`
   - 长期稳定性优于显式Euler

3. **约束满足**
   每步直接投影保证约束精确满足：
   ```
   |x_corrected - center| = radius （精确）
   ```

### 与传统方法对比

**传统力方法（拉格朗日乘数法）**：
```cpp
// 需要求解线性系统
// M·a = F_ext + J^T·λ
// J·a = -b
```

**PBD方法**：
```cpp
// 直接几何投影
x_new = project_to_constraint(x_predicted)
```

### 数值特性分析

1. **稳定性**
   - 无条件稳定（约束总是满足）
   - 大时间步长不会爆炸
   - 但可能有能量损失

2. **精度**
   - 一阶精度的速度更新
   - 对快速运动可能有数值阻尼
   - 通过子步（substeps）提高精度

3. **性能**
   ```javascript
   for (var step = 0; step < physicsScene.numSteps; step++) {
       // 使用1000个子步提高精度
   }
   ```
   - 子步数越多，越接近解析解
   - 计算成本线性增长

## 实际应用考虑

### 在UE/Unity中的对应实现

**Unreal Engine (Chaos)**：
```cpp
// Chaos中的类似实现
void FPBDRigidParticles::ApplyConstraints(float Dt)
{
    // 预测步
    P = X + V * Dt;
    
    // 约束投影
    for(auto& Constraint : Constraints)
    {
        Constraint.Apply(P, InvM);
    }
    
    // 速度更新
    V = (P - X) / Dt;
    X = P;
}
```

**Unity (Physics)**：
```csharp
// ConfigurableJoint的内部原理类似
joint.targetPosition = constraintPosition;
joint.xDrive.positionSpring = stiffness;
```

### 优化建议

1. **自适应子步**：
   ```javascript
   // 根据速度调整子步数
   var substeps = Math.ceil(vel.length() * adaptiveFactor);
   ```

2. **约束缓存**：
   ```javascript
   // 预计算约束参数
   this.invMass = 1.0 / mass;
   this.constraintStiffness = 1.0; // 可调节
   ```

3. **多约束处理**：
   ```javascript
   // Gauss-Seidel迭代
   for(let iter = 0; iter < iterations; iter++) {
       for(let constraint of constraints) {
           constraint.solve();
       }
   }
   ```

这种PBD实现方式的优雅之处在于：用简单的几何投影替代了复杂的力计算，同时保持了物理正确性。这也是为什么现代游戏引擎越来越多地采用基于位置的方法。