# 基于位置动力学 (Position Based Dynamics) 算法详解

## 1. 简介
传统的物理模拟通常是基于**力**的（Force Based Dynamics），即 $F=ma$。我们需要计算弹簧力、阻尼力，算出加速度，再积分得到速度和位置。这种方法在模拟刚性约束（如布料不可伸长）时，容易因为时间步长问题导致系统爆炸（不稳定）。

**Position Based Dynamics (PBD)** 提出了一种不同的思路：**直接操作位置**。我们先粗略预测粒子位置，然后通过投影（Projection）强行修正位置以满足约束条件。这种方法非常稳定、可控，且视觉效果好，广泛应用于游戏（如 NVIDIA PhysX）和电影特效中。

---

## 2. 算法核心流程

PBD 的模拟循环通常包含以下三个步骤：

### 第一步：预测 (Prediction / Integration)
首先，忽略内部约束（如布料拉扯），仅根据外部力（重力、风力）和惯性更新粒子位置。

我们使用 **Verlet 积分** 的变种。不显式存储速度 $v$，而是利用上一帧位置 $p_{old}$ 来推导速度。

$$
v \approx \frac{p - p_{old}}{\Delta t}
$$

预测新位置 $p_{new}$ 的公式：
$$
p_{new} = p + v \cdot \Delta t + \frac{F_{ext}}{m} \cdot \Delta t^2
$$

> **代码对应：** `Particle.integrate(dt)` 方法。

### 第二步：约束求解 (Constraint Solving)
这是 PBD 的灵魂。此时粒子可能跑得太远（布料被拉得无限长），我们需要修正它们的位置，使其满足约束条件。

**距离约束 (Distance Constraint)** 是最基础的约束：
$$
C(p_1, p_2) = |p_1 - p_2| - d_{rest} = 0
$$
其中 $d_{rest}$ 是两点间的自然长度。

如果当前距离 $d_{current} \neq d_{rest}$，我们需要移动 $p_1$ 和 $p_2$ 使得它们恢复距离。

**修正量计算：**
我们要把两点沿着连接线方向拉回。
$$
\Delta p = \frac{d_{current} - d_{rest}}{d_{current}} \cdot (p_2 - p_1)
$$

考虑到粒子的质量不同（例如固定点质量无穷大，不能移动），我们根据**质量倒数 ($w = 1/m$)** 来分配移动量：

对于粒子 1 的修正向量：
$$
\Delta p_1 = \frac{w_1}{w_1 + w_2} \cdot \Delta p
$$

对于粒子 2 的修正向量：
$$
\Delta p_2 = - \frac{w_2}{w_1 + w_2} \cdot \Delta p
$$

> **代码对应：** `Constraint.solve()` 方法。

### 第三步：迭代 (Iteration)
为了让系统更加坚硬（Stiff），我们在每一帧中重复执行“第二步”多次。
* 迭代次数少（1-2次）：布料看起来像橡胶，很有弹性。
* 迭代次数多（10-20次）：布料看起来像牛仔布或皮革，很难被拉伸。

---

## 3. 布料模型的构建

为了模拟布料，我们构建了一个粒子网格，包含三种类型的约束连接：

### A. 结构约束 (Structural Constraints)
* **连接方式**：水平和垂直相邻的粒子。
* **作用**：保持布料的基本形状，防止过度拉伸。
* **可视化**：在代码中开启 `Show Constraints` 看到的正方形网格。

### B. 抗剪切约束 (Shear Constraints)
* **连接方式**：连接对角线上的粒子 (Cross)。
* **作用**：防止布料发生剪切变形（即防止正方形变成菱形）。如果没有这个约束，布料会显得非常松散，没有质感。
* **代码实现**：连接 $(x, y)$ 和 $(x+1, y+1)$。

### C. 弯曲约束 (Bending Constraints)
* **连接方式**：连接相隔一个点的粒子（跨越连接，如 x 和 x+2）。
* **作用**：防止布料折叠得太厉害，用于模拟布料的抗弯刚度。
* *注：为了简化代码，本示例主要依赖结构和剪切约束来实现一定的弯曲阻力。*

---

## 4. 总结：为何选择 PBD？

| 特性 | 基于力 (Force Based) | 基于位置 (PBD) |
| :--- | :--- | :--- |
| **稳定性** | 差 (容易爆炸，对 dt 敏感) | **极好** (无条件稳定) |
| **刚度控制** | 难 (需要极小的 dt 或复杂的隐式积分) | **简单** (只需增加迭代次数) |
| **碰撞处理** | 复杂 (反弹力计算) | **简单** (直接投影位置) |
| **适用场景** | 高精度科学模拟 | **实时游戏、影视特效** |

# 新增功能：PBD 冲击互动原理

在基础的布料模拟之上，我们增加了用户通过鼠标左键点击布料，产生物理冲击波的功能。

## 1. 射线检测 (Raycasting)
要确定用户点在哪里，我们使用图形学标准的 Raycasting 技术：
1.  获取鼠标在屏幕上的 2D 坐标 (NDC空间, -1 到 1)。
2.  从摄像机位置向鼠标方向发射一条射线。
3.  检测射线与 `clothMesh` 的三角形是否有交点。
4.  如果有交点，获取交点坐标 `hitPoint` 和射线的方向 `rayDirection`。

## 2. 粒子筛选 (Proximity Search)
为了模拟真实的打击感，我们不能只推一个点，而是要推一个“区域”。
我们遍历所有粒子，计算它们与 `hitPoint` 的距离 $d$。
如果 $d < R_{impulse}$（冲击半径），则该粒子受到影响。

## 3. 施加冲量 (Applying Impulse)
在经典力学中，冲量 $J = \Delta p = m \Delta v$。
在 PBD 中，我们要**瞬间改变速度**。

回顾 PBD 速度公式：
$$
v = \frac{p_{curr} - p_{prev}}{\Delta t}
$$

如果我们想要给速度增加一个向量 $v_{add}$，我们不需要显式存储速度，而是修改 $p_{prev}$。
推导如下：
$$
v_{new} = v_{old} + v_{add}
$$
$$
\frac{p - p_{prev}^{new}}{\Delta t} = \frac{p - p_{prev}^{old}}{\Delta t} + v_{add}
$$
两边乘以 $\Delta t$ 并移项，得到修正公式：
$$
p_{prev}^{new} = p_{prev}^{old} - v_{add} \cdot \Delta t
$$

**代码实现：**
```javascript
// Particle 类中的方法
applyImpulse(forceVector, dt) {
    // forceVector 这里代表想要的瞬间速度变化量
    // 通过将 prevPos 往"反方向"移，下一帧计算出的 (pos - prevPos) 就会变大，从而产生巨大的速度
    this.prevPos.sub(forceVector.clone().multiplyScalar(dt));
}