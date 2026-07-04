# Session 7 — 调参完成

---

## 最终状态

小车可以在桌面上较小运动范围内持久平衡，存在轻微高频抖动。

| 项 | 当前值 |
|---|---|
| 工程目录 | `Template/` |
| 控制频率 | 200Hz，由 MPU6050 INT 触发 |
| 串口 | USART1 PA9/PA10, 115200, 8N1 |
| 目标 | 调参完成 |

---

## 最终参数

文件：[Template/HARDWARE/CONTROL/control.c](Template/HARDWARE/CONTROL/control.c)

```c
#define MECHANICAL_ZERO  -0.90f
#define SAFE_ANGLE_MAX   30.0f

float Balance_Kp = 380.0f;       // 直立环比例
float Balance_Kd = 23.50f;       // 直立环微分  Kd/Kp = 0.062

int   Velocity_Kp = 140;         // 速度环比例
float Velocity_Ki = 0.70f;       // 速度环积分

#define DEBUG_SENSOR_ONLY    0   // 正常驱动模式
```

文件：[Template/Drivers/BSP/MOTOR/motor.c](Template/Drivers/BSP/MOTOR/motor.c)

```c
#define MOTOR_LEFT_DZ       100
#define MOTOR_RIGHT_DZ      100
#define MOTOR_DZ_START      5
#define MOTOR_SAFE_LIMIT    7199
#define MOTOR_LEFT_GAIN   1.00f
#define MOTOR_RIGHT_GAIN  1.00f
```

死区逻辑（柔和补偿）：

```c
if (pwm > 5)       pwm += 100;
else if (pwm < -5) pwm -= 100;
else               pwm = 0;
```

---

## 控制架构

```
200Hz (5ms) 控制周期：

MPU6050 INT (PA12)
  → EXTI15_10_IRQHandler
    → exti.c: 互补滤波 (α=0.95) → pitch, gyro_y
    → control.c: Balance_Control()
      → 直立环 PD: balance_pwm = -(Kp × (pitch - MZ) + Kd × gyro_y)
      → 速度环 PI: speed_pwm = Vel_PI(speed_left + speed_right, target_speed=0)
      → final = PWM_Limit(balance_pwm + speed_pwm)
      → motorMove(final_left, final_right)
        → motor.c: 死区补偿 + SAFE_LIMIT → TIM1 PWM
```

Vel_PI 实现：[Template/HARDWARE/TIMER/gtim.c](Template/HARDWARE/TIMER/gtim.c)

- 增量式 PI：`ΔU = Kp × (ek - ek₋₁) + Ki × ek`
- 抗积分饱和：输出限幅到 PWM_MIN ~ PWM_MAX
- 电机关停时自动清零积分和历史误差

---

## Session 7 主要改动

### 1. 去除速度阻尼

`SPEED_DAMP_K` / `SPEED_DAMP_DEADZONE` 宏和 `speed_pwm = K × speed_sum` 速度阻尼计算被移除。速度阻尼是非积分的，无法真正消除稳态速度漂移。

### 2. 接入速度环 PI

`Vel_PI()` 已存在于 gtim.c 但从未被调用。在 control.c 中接入：

```c
velocity = speed_left + speed_right;
speed_pwm = Vel_PI(velocity, target_speed);
```

速度环 PI 输出叠加到直立环 PD 输出上，形成串级控制。

### 3. 去除 DEBUG_PWM_LIMIT 二次限幅

`DEBUG_PWM_LIMIT` 宏及其对 `final_left/right` 的钳位被移除，只保留 `PWM_Limit()` 和 motor.c 中的 `MOTOR_SAFE_LIMIT`。

### 4. OLED 显示变更

| 行 | 旧内容 | 新内容 |
|---|---|---|
| 1 | `Pitch: x.xx` | `U202513436WDZ` |
| 2 | `Speed: x` | `Pitch: x.xx` |
| 3 | `Batt: x.xxV` | 不变 |
| 4 | `Motor: ON/OFF` | 不变 |

---

## 调参过程回顾（Session 1–7）

### 关键里程碑

| 阶段 | Kp | Kd | 结果 |
|---|---|---|---|
| Session 5 初始 | 260 | 2.0 | 限幅 800，一倒就顶满 |
| Session 6 探索 | 240~350 | 1.35~7.0 | 发现 Kd/Kp < 0.018 无法收敛 |
| Session 7 前期 | 280~350 | 5.5~6.0 | 站立 6~7s，振幅缓慢增长 |
| **Session 7 最终** | **380** | **23.5** | 持久平衡，小幅高频抖动 |

### 核心发现

1. **Kd/Kp 比值是本车最关键参数**。Kd 从 1.8 逐步提到 23.5（比值从 0.005 → 0.062）后，振荡从不衰减变为持久平衡。
2. **速度阻尼不是真正的速度环**。`SPEED_DAMP_K × speed_sum` 只能抑制快速跑远，无法消除慢速漂移。接入增量式 PI 后车的位置漂移被有效抑制。
3. **PWM 限幅解除是关键一步**。早期 `MOTOR_SAFE_LIMIT=800` 卡死了控制器的出力。
4. **左编码器始终为 0**，硬件问题未修复。速度环只有右编码器数据，但不影响基本平衡。

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
| `ss` | `sl + sr` |
| `bp` | 直立环 PD 输出 |
| `sp` | 速度环 PI 输出 |
| `fl/fr` | 进入 motorMove 前的目标 PWM |
| `ml/mr` | 死区补偿后的实际 PWM |
| `en` | 电机开关 |
| `be` | 平衡使能 |

---

## 修改过的文件

```
Template/
├── User/main.c                     ← OLED 显示内容变更
├── HARDWARE/CONTROL/control.c      ← PID 参数、去除速度阻尼和 DEBUG_PWM_LIMIT、接入速度环 PI
└── Drivers/BSP/MOTOR/motor.c       ← 死区参数
```

---

## 遗留问题

1. **左编码器**：`sl` 始终为 0，PA0/PA1 或 TIM2 硬件问题。修复后速度环精度会提升。
2. **高频抖动**：当前有小幅高频抖动，可尝试微调 Kd（23.5 → 25~28）或 MECHANICAL_ZERO。

---

*Session 7 结束。调参工作完成。*
