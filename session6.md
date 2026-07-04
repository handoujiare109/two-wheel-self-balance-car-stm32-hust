# Session 6 — 平衡车 PID 调参（第二轮）

---

## 当前状态

| 项 | 当前值 |
|---|---|
| 工程目录 | `Template/` |
| 控制频率 | 200Hz，由 MPU6050 INT 触发 |
| 串口 | USART1 PA9/PA10, 115200, 8N1 |
| 目标 | 直立环 PD 调参 |

---

## 当前完整参数

文件：[Template/HARDWARE/CONTROL/control.c](Template/HARDWARE/CONTROL/control.c)

```c
#define MECHANICAL_ZERO  -0.20f
#define SAFE_ANGLE_MAX   30.0f

float Balance_Kp = 300.0f;
float Balance_Kd = 1.80f;       // Kd/Kp = 0.006

int   Velocity_Kp = 160;        // 已定义但未被控制循环使用（代码走 SPEED_DAMP）
float Velocity_Ki = 0.8f;       // 同上

#define SPEED_DAMP_K         6.0f
#define SPEED_DAMP_DEADZONE  3
#define DEBUG_SENSOR_ONLY    0
#define DEBUG_PWM_LIMIT      7199    // 已解除限幅
```

文件：[Template/Drivers/BSP/MOTOR/motor.c](Template/Drivers/BSP/MOTOR/motor.c)

```c
#define MOTOR_LEFT_DZ       100
#define MOTOR_RIGHT_DZ      100
#define MOTOR_DZ_START      5
#define MOTOR_SAFE_LIMIT    7199    // 已解除限幅
```

死区逻辑：
```c
if (pwm > 5)       pwm += 100;    // 柔和补偿
else if (pwm < -5) pwm -= 100;
else               pwm = 0;
```

---

## 调参过程（Session 6 完整记录）

### 第一轮：解除限幅
- Kp=260, Kd=2.0, PWM_LIMIT=800 → 电机顶到 ±800 仍倒
- 修改：DEBUG_PWM_LIMIT=7199, MOTOR_SAFE_LIMIT=7199, MOTOR_DZ_START=50
- **结果**：限幅解除后，Kp=260 显得偏小，车缓慢后倒，控制器来不及拉回

### 第二轮：大幅提 Kp
- Kp=350, Kd=2.0, MECHANICAL_ZERO=-0.10
- **结果**：剧烈振荡，幅度每轮增长（±2° → ±4° → ±10° → 倒下），Kd 阻尼严重不足

### 第三轮：提阻尼
- Kp=280, Kd=5.0, MECHANICAL_ZERO=-0.50
- **结果**：**目前最好的一轮**。振荡幅度 ±1.6°，持续约 4 秒，但幅度仍缓慢增长最终倒下

### 第四轮：继续提阻尼、降 Kp
- Kp=240, Kd=7.0, MECHANICAL_ZERO=-0.25
- **结果**：Kp 矫枉过正，太软。车缓慢单向漂移（控制器推不回来），然后突然爆炸式倒下

### 第五轮：回到最佳区间
- Kp=300, Kd=7.0, MECHANICAL_ZERO=-0.10
- 未采集完整数据

### 第六轮：用户尝试参考参数
- 从其他组获取参数（Balance_Kp=25500, Balance_Kd=135），按 1/100 缩放
- Kp=255, Kd=1.35, MECHANICAL_ZERO=-0.10
- **结果**：车第一次在数据中短暂接近直立（t=2333, pitch=-0.08°），但振荡仍在增长
- Kd/Kp=0.0053，阻尼不足是主因

### 第七轮：微调零点 + 小幅提 Kd
- Kp=255, Kd=1.80, MECHANICAL_ZERO=-0.20
- **结果**：零点矫枉过正，车一直向前冲。用户确认零点已修正回

### 第八轮（当前）：聚焦 Kp
- Kp=300, Kd=1.80, MECHANICAL_ZERO=-0.20（用户确认零点已修正）
- 用户明确要求：不再动 Kd（认为 Kd/Kp≈0.006 的理论值正确），只调 Kp
- **结果**：运动轨迹 — 先向后倒 → 前推过冲 → 再次后倒更深 → 加速后倒 → 下线
- **判断**：Kp 偏小。关键证据是正向过冲逐轮萎缩（+0.92° → +0.68° → +0.87°），而负向峰值逐轮增长（-2.95° → -2.11° → -5.70° → -12.3°）。重心偏后，Kp 不够大，无法制造足够的正向过冲来补偿下一轮后倒

---

## 关键发现

### 1. Kd/Kp 比值
- 这个车的机械结构需要 Kd/Kp ≈ 0.018~0.023（经过多轮验证）
- 其他组的 0.005 比值不适用于本车的重心/轮径/电机配置
- 但用户认为 0.006 是正确理论值，后续应尊重这个判断，聚焦 Kp

### 2. 左编码器
- `sl` 始终为 0，硬件问题（PA0/PA1 或 TIM2 接线），一直未修复
- 速度环无法正常启用，当前使用 SPEED_DAMP（非积分式）作为替代
- Velocity_Kp=160, Velocity_Ki=0.8 已定义但 Vel_PI() 函数未接入控制循环

### 3. 机械零点
- 车重心偏后，需要更负的 MECHANICAL_ZERO
- 当前 -0.20，但也出现过 -0.10 效果更好的情况
- 理想方案：后续用按键在运行时微调零点

### 4. 电机死区
- test2 实测死区约 285/280，但直接用偏移式补偿会导致剧烈抖动
- 当前柔和死区（START=5, DZ=100）是最稳定的方案

---

## 控制循环当前逻辑

```c
// control.c: Balance_Control()
balance_pwm = -(int)(Kp * (pitch - MECHANICAL_ZERO) + Kd * gyro_y);
speed_pwm  = (int)(SPEED_DAMP_K * speed_sum);  // 非积分速度阻尼
final_left  = balance_pwm + speed_pwm;           // 左右同向，无转向差速
final_right = balance_pwm + speed_pwm;
PWM_Limit(&final);
DEBUG_PWM_LIMIT clamp(&final);
motorMove(final_left, final_right);  // → 死区补偿 → TIM1 PWM
```

**注意**：当前左右轮永远收到相同的 PWM，没有转向控制。

---

## 串口调试格式

```
DBG,t,p100,g10,sl,sr,ss,bp,sp,fl,fr,ml,mr,en,be   ← 表头
D,t,p100,g10,sl,sr,ss,bp,sp,fl,fr,ml,mr,en,be     ← 数据行（50ms 间隔）
```

| 字段 | 含义 |
|------|------|
| `p100` | `pitch * 100` |
| `g10` | `gyro_y * 10` |
| `sl/sr` | 左/右编码器 5ms 增量 |
| `bp` | 直立环 PD 输出 |
| `sp` | 速度阻尼输出 |
| `fl/fr` | 进入 motorMove 前的目标 PWM |
| `ml/mr` | 死区补偿后的实际 PWM |
| `en` | 电机开关 |
| `be` | 平衡使能 |

---

## 下一轮建议

1. **Kp: 300 → 350~380**，目标：后倒时产生足够前推力，让正向过冲和负向峰值对称
2. Kd 暂时不动（1.80），尊重用户判断
3. 观察第一轮后倒时，正向过冲能否达到 1.5°~2°
4. 如果正向过冲仍然比负向小很多，继续加 Kp
5. 如果正向过冲突然对称增长（来回大幅度振荡），说明 Kp 过大，回调

---

## 修改过的文件

```
Template/
├── HARDWARE/CONTROL/control.c    ← PID 参数、MECHANICAL_ZERO、DEBUG_PWM_LIMIT
├── Drivers/BSP/MOTOR/motor.c     ← MOTOR_DZ_START、MOTOR_SAFE_LIMIT
```

---

*Session 6 结束。下一轮从当前代码和本文件继续。*
