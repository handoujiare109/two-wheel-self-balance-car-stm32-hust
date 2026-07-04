# 平衡车完整工程 — 文档

---

## 工程结构

```
Template/
├── Drivers/
│   ├── CMSIS/Include/              ← core_cm3.h 等 CMSIS 核心头文件
│   ├── CMSIS/Device/ST/STM32F1xx/  ← stm32f103xb.h + system_stm32f1xx
│   ├── STM32F1xx_HAL_Driver/       ← HAL 库（仅保留需要的模块）
│   └── BSP/                        ← 板级外设驱动
│       ├── LED/        led.c/h     ← PA4 LED
│       ├── KEY/        key.c/h     ← PA5 按键消抖
│       ├── EXTI/       exti.c/h    ← PA12 MPU6050 INT + PA5 KEY2
│       ├── MOTOR/      motor.c/h   ← TIM1 PWM + TB6612 方向控制
│       ├── ENCODER/    encoder.c/h ← TIM2(左) + TIM4(右) 编码器
│       ├── MPU6050/    atk_ms6050  ← 软件 I2C MPU6050 驱动
│       ├── ADC/        adc.c/h     ← PB0 电池电压（寄存器版）
│       └── OLED/       oled.c/h    ← 4线SPI OLED 128×64（寄存器版）
├── SYSTEM/
│   ├── sys/           sys.c/h      ← 时钟初始化（调参阶段强制 HSI+PLL 64MHz）
│   ├── delay/         delay.c/h    ← SysTick us/ms 延时
│   └── usart/         usart.c/h    ← USART1 printf 重定向
├── HARDWARE/
│   ├── TIMER/         gtim.c/h     ← TIM3 5ms 定时 + Vel_PI() 速度环
│   └── CONTROL/       control.c/h  ← ★ 串级 PID 平衡控制
├── Startup/           startup_stm32f103xb.s
├── User/
│   ├── main.c                      ← 初始化 + 主循环（OLED/串口 50ms 刷新）
│   ├── stm32f1xx_hal_conf.h        ← HAL 模块开关
│   └── stm32f1xx_it.c              ← SysTick / 中断入口
└── Output/                          ← 编译输出
```

---

## 引脚分配（最终版）

| 功能 | GPIO | 外设 | 备注 |
|------|------|------|------|
| LED | PA4 | GPIO 输出 | |
| KEY2 | PA5 | EXTI9_5 | 启停电机，30ms 去抖 |
| 左电机 PWM | PA8 | TIM1_CH1 | 10KHz |
| 左电机 AIN1 | PB14 | GPIO | TB6612 方向 |
| 左电机 AIN2 | PB15 | GPIO | TB6612 方向 |
| 右电机 PWM | PA11 | TIM1_CH4 | 10KHz |
| 右电机 BIN1 | PB13 | GPIO | TB6612 方向 |
| 右电机 BIN2 | PB12 | GPIO | TB6612 方向 |
| 左编码器 A/B | PA0/PA1 | TIM2_CH1/CH2 | 正交编码 |
| 右编码器 A/B | PB6/PB7 | TIM4_CH1/CH2 | 正交编码 |
| MPU6050 SCL | PB8 | 软件 I2C | |
| MPU6050 SDA | PB9 | 软件 I2C | |
| MPU6050 AD0 | PA15 | GPIO | 接 GND, 地址 0x68 |
| MPU6050 INT | PA12 | EXTI15_10 | 200Hz 控制触发 |
| 电池 ADC | PB0 | ADC1_IN8 | 电阻分压 |
| OLED RST | PB3 | GPIO | 寄存器驱动 |
| OLED CS | PA7 | GPIO | |
| OLED DC/RS | PA15 | GPIO | 与 AD0 共用 |
| OLED SCK | PB5 | GPIO | |
| OLED MOSI | PB4 | GPIO | |
| USART1 TX/RX | PA9/PA10 | USART1 | 115200 波特率 |

> PA15 = OLED_RS + MPU6050_AD0 双用途。AD0 仅上电时采样（低→0x68）。代码调用 `__HAL_AFIO_REMAP_SWJ_NOJTAG()` 释放 PA15/PB3/PB4。

---

## 定时器分配

| 定时器 | 用途 | 关键参数 |
|--------|------|------|
| TIM1 | 电机 PWM (CH1=PA8, CH4=PA11) | Period=7199, PSC=0 → 10KHz |
| TIM2 | 左编码器 (CH1=PA0, CH2=PA1) | 正交编码, Period=65535 |
| TIM3 | 5ms 控制定时器 | Period=49, PSC 根据 SystemCoreClock 自动计算 → 200Hz |
| TIM4 | 右编码器 (CH1=PB6, CH2=PB7) | 正交编码, Period=65535 |

---

## 控制架构

```
200Hz (5ms) 控制周期：

MPU6050 INT (PA12)
  → EXTI15_10_IRQHandler
    → exti.c: 互补滤波 (α=0.95)
      → pitch, gyro_y
    → control.c: Balance_Control()
      → 直立环 PD: balance_pwm = -(Kp × (pitch - MZ) + Kd × gyro_y)
      → 速度环 PI: speed_pwm = Vel_PI(speed_left + speed_right, target_speed)
      → motorMove(final_left, final_right)
        → motor.c: 死区 + 增益校准 → TIM1 PWM
```

### 当前 PID 参数（[control.c](HARDWARE/CONTROL/control.c)）

```c
#define MECHANICAL_ZERO  -0.90f
#define SAFE_ANGLE_MAX   30.0f
float Balance_Kp = 380.0f;
float Balance_Kd = 23.50f;
int   Velocity_Kp = 140;
float Velocity_Ki = 0.70f;
#define DEBUG_SENSOR_ONLY    0
```

### 电机死区（[motor.c](Drivers/BSP/MOTOR/motor.c)）

```c
#define MOTOR_LEFT_DZ       100
#define MOTOR_RIGHT_DZ      100
#define MOTOR_DZ_START      5
#define MOTOR_SAFE_LIMIT    7199
#define MOTOR_LEFT_GAIN   1.00f
#define MOTOR_RIGHT_GAIN  1.00f
```

---

## MDK 工程配置

### 1. Options for Target

| 选项卡 | 配置项 | 值 |
|--------|--------|-----|
| Target | XTAL | 8.0 MHz |
| Output | Create HEX File | 勾选 |
| Output | Select Folder for Objects | `Output/` |
| C/C++ | Define | `USE_HAL_DRIVER,STM32F103xB` |
| C/C++ | Optimization | Level 1 (-O1) |
| C/C++ | C99 mode | 勾选 |
| Target | MicroLIB | 勾选 |

### 2. 头文件包含路径

```
..\Drivers\CMSIS\Include
..\Drivers\CMSIS\Device\ST\STM32F1xx\Include
..\Drivers\STM32F1xx_HAL_Driver\Inc
..\Drivers\BSP\LED
..\Drivers\BSP\KEY
..\Drivers\BSP\EXTI
..\Drivers\BSP\MOTOR
..\Drivers\BSP\ENCODER
..\Drivers\BSP\MPU6050
..\Drivers\BSP\ADC
..\Drivers\BSP\OLED
..\SYSTEM\sys
..\SYSTEM\delay
..\SYSTEM\usart
..\HARDWARE\TIMER
..\HARDWARE\CONTROL
..\User
```

### 3. 工程分组

| 分组 | 文件 |
|------|------|
| Startup | `Startup/startup_stm32f103xb.s` |
| SYSTEM | `sys.c`, `delay.c`, `usart.c` |
| User | `main.c`, `stm32f1xx_it.c` |
| HARDWARE | `gtim.c`, `control.c` |
| BSP | `led.c`, `key.c`, `exti.c`, `motor.c`, `encoder.c`, `atk_ms6050.c`, `atk_ms6050_iic.c`, `adc.c`, `oled.c` |
| STM32F1xx_HAL_Driver | `stm32f1xx_hal.c`, `hal_rcc.c`, `hal_gpio.c`, `hal_cortex.c`, `hal_tim.c`, `hal_uart.c`, `hal_exti.c`, `hal_dma.c`, `hal_flash.c`, `hal_pwr.c`, `hal_adc.c`, `hal_spi.c` |
| CMSIS | `system_stm32f1xx.c` |

---

## 调参指南

调参已完成，详见项目根目录的 [session7.md](../session7.md)，包含完整调参过程和最终参数。

**核心原则**：

1. 先直立环 PD → 松手能站 → 再速度环 PI
2. 一次只改一个参数
3. 高频抖动 → 微调 Kd；低频晃动 → 升 Kd
4. Kd/Kp 比值是本车最关键参数，最终比值约 0.062

---

## 串口调试格式

USART1：PA9/PA10，`115200, 8N1, 无流控`。

启动后输出表头：

```text
DBG,t,p100,g10,sl,sr,ss,bp,sp,fl,fr,ml,mr,en,be
```

每 50ms 输出一行：

```text
D,t,p100,g10,sl,sr,ss,bp,sp,fl,fr,ml,mr,en,be
```

字段说明：

| 字段 | 含义 |
|------|------|
| `t` | `HAL_GetTick()` 毫秒 |
| `p100` | `pitch * 100`，例如 `75` 表示 `0.75°` |
| `g10` | `gyro_y * 10`，例如 `-25` 表示 `-2.5°/s` |
| `sl/sr` | 左/右编码器 5ms 增量 |
| `ss` | `sl + sr` |
| `bp` | 直立环 PD 输出 |
| `sp` | 速度阻尼输出 |
| `fl/fr` | 进入 `motorMove()` 前的左右目标 PWM |
| `ml/mr` | 死区补偿和限幅后的实际左右 PWM |
| `en` | 电机开关 `motor_enable` |
| `be` | 平衡使能 `balance_enable` |

为兼容 Keil MicroLIB，串口不输出浮点数，全部用整数缩放。

## 调试工具

- **test2/** — 电机死区测试程序，PWM 280-320 精细扫描
- **串口 115200** — CSV 格式见上节
- **OLED** — 4 行：U202513436WDZ / Pitch / Batt / Motor
