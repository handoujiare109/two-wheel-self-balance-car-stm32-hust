# Session 4 — 平衡车项目调参阶段

---

## 项目状态

| 项 | 值 |
|---|---|
| 芯片 | STM32F103C8T6 |
| 环境 | Keil MDK 5.36 + ARM Compiler 5 (AC5) |
| 库 | STM32F1xx HAL，部分模块寄存器级 |
| 时钟 | 自动回退：HSE+PLL(72MHz) → HSI+PLL(64MHz) |
| 当前状态 | 所有驱动正常，进入 PID 调参阶段 |
| 传感器 | MPU6050 稳定，pitch 在 0.01° 量级波动 |

---

## 当前极简配置

已移除所有实验性改动，回到最小可用基线：

| 组件 | 配置 | 说明 |
|------|------|------|
| MPU6050 DLPF | 默认 98Hz | 无手动覆盖 |
| 互补滤波 α | 0.95 | 标准参数，τ≈95ms |
| 直立环 | Kp=250, Kd=0.5 | 纯 PD，无滤波 |
| 速度环 | Vel_Kp=0, Vel_Ki=0 | 关闭 |
| 死区补偿 | DZ=0 | 关闭，无补偿 |
| 安全关停 | ±35° | 超限关电机 |

---

## 可调参数速查表

### 一、直立环 PD（[control.c](Template/HARDWARE/CONTROL/control.c)）

| 参数 | 当前值 | 作用 | 调大 | 调小 |
|------|--------|------|------|------|
| `Balance_Kp` | 250.0 | 角度比例增益 | 恢复力更强，但可能振荡 | 更柔和，但可能无力 |
| `Balance_Kd` | 0.5 | 角速度微分增益 | 阻尼更强，抑振荡 | 阻尼弱，可能晃/抖 |
| `MECHANICAL_ZERO` | -0.25 | 机械零点偏移(°) | — | — |

**Kp 和 Kd 的关系**：Kd/Kp ≈ 0.002~0.01 是常见范围。Kd 太高会放大 gyro 噪声 → 高频抖动；Kd 太低 → 低频晃动 → 振荡发散 → 猛冲。

**调参顺序**：先固定 Kd=0，调 Kp 到松手能短暂站住 → 再加 Kd 消除晃动 → 反复微调。

### 二、速度环 PI（[control.c](Template/HARDWARE/CONTROL/control.c)）

| 参数 | 当前值 | 作用 |
|------|--------|------|
| `Velocity_Kp` | 0 | 速度误差比例 |
| `Velocity_Ki` | 0.0 | 速度误差积分 |

**注意**：直立环调稳之前必须为 0！否则和直立环打架。

速度环启用时建议：Vel_Kp 从 20 开始，Vel_Ki = Vel_Kp / 200。

### 三、电机死区（[motor.c](Template/Drivers/BSP/MOTOR/motor.c)）

| 参数 | 当前值 | 作用 |
|------|--------|------|
| `MOTOR_LEFT_DZ` | 0 | 左电机死区补偿 |
| `MOTOR_RIGHT_DZ` | 0 | 右电机死区补偿 |

**死区补偿有两种模式**（在 `motor_deadzone()` 函数中切换）：

**硬阈值模式**：
```c
if (*pwm > 0 && *pwm < dz)  *pwm = dz;   // 低于 DZ 强制拉到 DZ
```
- 优点：不产生过度响应
- 缺点：低于 DZ/Kp 的角度不响应（如 DZ=200,Kp=200 → 1° 死区）

**偏移模式**：
```c
if (*pwm > 0)  *pwm += dz;   // 非零就叠加 DZ 偏移
```
- 优点：小角度有响应，无硬跳变
- 缺点：微角度可能过度响应

**选择指南**：小角度无响应→用偏移；高频微抖→用硬阈值；都不行→DZ=0。

### 四、电机增益（[motor.c](Template/Drivers/BSP/MOTOR/motor.c)）

| 参数 | 当前值 | 作用 |
|------|--------|------|
| `MOTOR_LEFT_GAIN` | 1.00 | 左电机 PWM 倍率 |
| `MOTOR_RIGHT_GAIN` | 1.00 | 右电机 PWM 倍率 |

两侧电机转速不一致时调整。如左轮偏慢 → `MOTOR_LEFT_GAIN = 1.05`。

### 五、MPU6050 传感器（[exti.c](Template/Drivers/BSP/EXTI/exti.c) + [atk_ms6050.c](Template/Drivers/BSP/MPU6050/atk_ms6050.c)）

| 参数 | 位置 | 当前值 | 作用 |
|------|------|--------|------|
| 互补滤波 α | exti.c:73 | 0.95 | 越大越信任陀螺仪(平滑但滞后)，越小越信任加速度计(快速但噪声大) |
| DLPF 带宽 | 调用 `atk_ms6050_set_lpf()` | 默认 98Hz | 42Hz 可滤电机振动，20Hz 更强但延迟 10ms |
| 采样率 | `atk_ms6050_set_rate(200)` | 200Hz | 决定控制频率，改此值需同步改 TIM3 周期 |
| 陀螺仪量程 | atk_ms6050.c 初始化 | ±2000°/s | 16.4 LSB/(°/s) |
| 加速度计量程 | atk_ms6050.c 初始化 | ±2g | 用于 atan2f 计算倾角 |
| 轴组合 | exti.c:70-71 | AY + GX | **不可随意改**，由 MPU6050 安装方向决定 |

**互补滤波公式**：
```
comp_angle = α × (comp_angle + gyro_y × Δt) + (1-α) × accel_angle
pitch = comp_angle
```
- α=0.95 → τ≈95ms → 滞后大但平滑
- α=0.90 → τ≈45ms → 中等
- α=0.85 → τ≈28ms → 快速但噪声更多

### 六、定时器配置

| 定时器 | 文件 | 用途 | 关键参数 |
|--------|------|------|------|
| TIM1 | motor.c | 电机 PWM | Period=7199, PSC=0 → 10KHz |
| TIM2 | encoder.c | 左编码器 | 正交编码模式, Period=65535 |
| TIM3 | gtim.c | 5ms 控制定时 | Period=49, PSC=7199 → 200Hz |
| TIM4 | encoder.c | 右编码器 | 正交编码模式, Period=65535 |

### 七、限幅与安全

| 参数 | 位置 | 值 | 作用 |
|------|------|----|------|
| `PWM_MAX` | motor.h | 7199 | PWM 上限 |
| `PWM_MIN` | motor.h | -7199 | PWM 下限 |
| `SAFE_ANGLE_MAX` | control.c | 35.0° | 超此倾角自动关停电机 |
| 低压保护 | exti.c:84 | 暂时绕过 | `balance_enable = 1` 始终为真 |

### 八、时钟与系统

| 参数 | 文件 | 说明 |
|------|------|------|
| HSE 晶振 | sys.c | 优先使用 8MHz HSE ×9 = 72MHz |
| HSI 回退 | sys.c | HSE 失败→HSI/2×16 = 64MHz |
| delay_init | main.c | `SystemCoreClock / 1000000` 自动适配 |

---

## 已修改文件清单（Session 1-4 累计）

```
Template/
├── User/main.c                     ← 初始化 + 主循环 + 桩函数
├── SYSTEM/sys/sys.c                ← HSE 回退 HSI+PLL
├── HARDWARE/CONTROL/control.c      ← ★ 串级 PID + 安全关停
├── HARDWARE/CONTROL/control.h      ← extern 声明
├── HARDWARE/TIMER/gtim.c           ← TIM3 5ms + Vel_PI
├── Drivers/BSP/OLED/oled.c         ← 寄存器 GPIO
├── Drivers/BSP/OLED/oled.h         ← 寄存器宏
├── Drivers/BSP/EXTI/exti.c         ← ★ 互补滤波 + 轴组合 + 消抖
├── Drivers/BSP/MOTOR/motor.c       ← ★ 死区补偿 + 增益校准
├── Drivers/BSP/ENCODER/encoder.c   ← TIM4 右编码器
├── Drivers/BSP/ENCODER/encoder.h   ← 新增声明
├── Drivers/BSP/MPU6050/atk_ms6050.c ← INT 使能 + 200Hz
test2/
├── main.c                          ← 电机死区测试程序
├── test2.uvprojx                   ← Keil 工程
└── readme.md                       ← 使用说明
```

---

## 调试技巧

### OLED 确认存活
OLED 显示 4 行：Pitch / Speed / Batt / Motor。如果显示乱码 → 芯片已复位但 OLED 未初始化（正常，重新烧录主程序即可）。

### 串口输出格式（115200 波特率）
```
P=2.3 G=0.5 V=12.3 S=15 E=1
│     │       │       │    └─ 平衡使能
│     │       │       └────── 编码器速度和
│     │       └────────────── 电池电压
│     └────────────────────── 陀螺仪角速度
└──────────────────────────── 车身倾角
```

### 关键注意
- `pitch` 前倾应为正值，后倾负值
- `gyro_y` 前倾角速度应为正值
- 按 KEY2(PA5) 启停电机
- **浮点数常量必须写 `250.0f`，不能写 `250f`**（Keil AC5 报错）
- 烧录后芯片需断电重启（可能 HAL 状态残留）
- PB3/PB4/PA15 是 JTAG 引脚，oled.c 已释放

---

## 快速恢复

换电脑或重开 Keil 后：
1. 打开 `Template/final.uvprojx`
2. 确认宏：`USE_HAL_DRIVER,STM32F103xB`
3. Optimization: Level 1 (-O1), C99 mode, MicroLIB
4. F7 编译 → F8 烧录 → 小车断电重启 → 按 KEY2

---

## 调参经验总结

1. **一次只改一个参数**，观察效果后再改下一个
2. **先直立环，后速度环**：速度环没关闭时和直立环打架是常见问题
3. **高频抖 vs 低频晃**：高频(电机嗡嗡响)→Kd 太大或 gyro 噪声；低频(不倒翁)→Kd 太小
4. **机械零点不准** → 松手后向一个方向持续加速
5. **死区选择**：偏移式→小角度响应好但可能微抖；硬阈值→干净但有死区；DZ=0→依赖自然摩擦
6. **传感器滤波**：DLPF 降低可滤振动但增加延迟；α 降低可加速响应但引入噪声

---

*Session 4 结束。下一步：在极简基线(Kp=250, Kd=0.5, DZ=0)上继续调参。*