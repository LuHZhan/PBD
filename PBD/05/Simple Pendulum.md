# 物理模拟

`AnalyticBead` 中的 `simulate` 方法实现的是**圆周约束下的单摆运动**的解析解，这确实是真实世界的物理。让我详细解释每个公式：

## 1. 角加速度方程

```javascript
var acc = -gravity / this.radius * Math.sin(this.angle);
```

这是**单摆的角加速度公式**：

$$\alpha = -\frac{g}{r} \sin\theta$$

### 推导过程：

- 重力的切向分量：$F_{\text{tangential}} = mg\sin\theta$
- 切向加速度推导：
- $$F_t = m × a_tangential$$
- $$mg sin θ = m × a_tangential$$
- $$a_tangential = g sin θ $$
- 切向加速度：$a_{\text{tangential}} = g\sin\theta$
- 角加速度：$\alpha = \frac{a_{\text{tangential}}}{r} = \frac{g\sin\theta}{r}$
- 负号表示恢复力的方向（总是指向平衡位置）

## 2. 数值积分（欧拉方法）

```javascript
this.omega += acc * dt;        // ω = ω₀ + α·dt
this.angle += this.omega * dt; // θ = θ₀ + ω·dt
```

这是简单的欧拉积分，用于更新角速度和角度。

## 3. 约束力计算

```javascript
// 为什么m消失了，原因1：简化计算
// 如果质量是常数（通常是），在比较力的大小或做动力学计算时，可以先不考虑质量
// 需要实际力值时，再乘以质量即可
var centrifugalForce = this.omega * this.omega * this.radius;
var force = centrifugalForce + Math.cos(this.angle) * Math.abs(gravity);
```

这计算的是**绳子/轨道对珠子的约束力**（法向力）：

$$F_{\text{constraint}} = m(\omega^2 r + g\cos\theta)$$

### 物理含义：

- **第一项** $m\omega^2 r$：向心力需求（维持圆周运动）
- **第二项** $mg\cos\theta$：重力的径向分量

### 为什么这样计算？

在圆周约束下，约束力必须：

1. 维持约束：确保珠子始终在圆形轨道上运动
2. 提供向心加速度：补偿圆周运动所需的向心力
3. 平衡重力分量：抵消重力在径向的分量

## 与PBD方法的对比

这个demo精妙地展示了两种方法计算同一个物理量（约束力）：

### 解析方法（AnalyticBead）：

```javascript
// 基于牛顿力学的分析
// 1. 计算运动学
// 2. 从运动学推导约束力
force = m·ω²·r + mg·cos(θ)
```

### PBD方法（Bead）：

```javascript
// 基于几何投影
var lambda = physicsScene.wireRadius - len;  // 位置修正量
this.pos.add(dir, lambda);                    // 投影回圆周
force = Math.abs(lambda / sdt / sdt);         // 从修正量反推力
```

PBD通过**拉格朗日乘数** λ 来计算约束力： $$F = \frac{\lambda}{\Delta t^2}$$

## 验证物理正确性

这个demo的关键是：**两种完全不同的方法应该得到相同的约束力值**！

- 红色珠子（PBD）：通过几何投影
- 绿色珠子（解析）：通过物理公式

当你运行代码时，两个力值应该非常接近，这验证了：

1. PBD方法虽然是几何的，但物理上是正确的
2. 随着时间步长减小（numSteps增加），PBD结果会收敛到解析解

## 实际意义

在Chaos物理引擎中，类似的思想被用于：

```cpp
// Chaos中的约束力计算
float FPBDConstraint::CalculateConstraintForce(float Lambda, float Dt) {
    // λ 是位置修正量
    // 约束力 = λ / Δt²
    return Lambda / (Dt * Dt);
}
```

这种方法的优势：

- **不需要显式计算加速度**
- **自动满足约束条件**
- **数值稳定性更好**

这就是为什么游戏物理引擎普遍采用PBD/XPBD方法的原因！

让我详细解释 `getPos()` 方法中角度转换为笛卡尔坐标的原理。

# getPose()坐标系定义

首先要理解这个特殊的坐标系设置：

### 角度定义
- **angle = 0**：珠子在**底部**（6点钟位置）
- **angle = π/2**：珠子在**右侧**（3点钟位置）  
- **angle = π**：珠子在**顶部**（12点钟位置）
- **angle = 3π/2**：珠子在**左侧**（9点钟位置）

### 为什么是这样的坐标系？

这是一个**从底部开始，逆时针增长**的角度系统，标准极坐标（从右侧开始，逆时针增长）

## 坐标转换推导

### 1. 标准极坐标（从右侧开始）
```
x = r·cos(θ)
y = r·sin(θ)
```

### 2. 从底部开始的坐标系
需要将角度旋转 90 度（π/2）：
- 当 angle = 0 时，实际对应标准角度 = -π/2（指向下方）
- 当 angle = π/2 时，实际对应标准角度 = 0（指向右方）

### 3. 数学推导

从底部开始的角度 `angle` 与标准角度 `θ_standard` 的关系：
```
θ_standard = angle - π/2
```

代入标准极坐标公式：
```
x = r·cos(angle - π/2)
y = r·sin(angle - π/2)
```

使用三角恒等式：
```
cos(α - π/2) = sin(α)
sin(α - π/2) = -cos(α)
```

得到：
```
x = r·sin(angle)
y = -r·cos(angle)
```

## 为什么选择这个坐标系？

1. **物理直观**：angle = 0 对应珠子自然悬挂的位置（底部）
2. **单摆类比**：与传统单摆分析一致，θ = 0 是平衡位置
3. **重力方向**：便于分析重力分量
   - 切向分量：mg·sin(angle)
   - 法向分量：mg·cos(angle)

## 图示说明

```
        π (顶部)
         |
         ↑
    3π/2 ← → π/2
   (左)   ↓   (右)
         |
        0 (底部)
        
角度增加方向：顺时针
坐标系：y轴向上为正
```
## Bead PBD模拟


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
