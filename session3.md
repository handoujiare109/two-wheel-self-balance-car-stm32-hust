# Session 3 — 平衡车项目 Debug 完成 & 调参阶段

---

## 项目概述

| 项 | 值 |
|---|---|
| 芯片 | STM32F103C8T6 |
| 环境 | Keil MDK 5.36 + ARM Compiler 5 (AC5) |
| 库 | STM32F1xx HAL，部分模块寄存器级 |
| 当前状态 | ✅ 所有驱动正常，OLED 显示正常，进入 PID 调参阶段 |
| 代码大小 | ~30600 bytes (Code) + 1832 (RO) |
| 时钟 | 自动回退：HSE(72MHz) → HSI+PLL(64MHz) |

---

## 当前可以工作的功能

| 功能 | 状态 | 备注 |
|------|------|------|
| OLED 显示 | ✅ | 寄存器级驱动，4 行数据 (Pitch/Speed/Batt/Motor) |
| MPU6050 | ✅ | INT 200Hz 触发，AY+GX 轴组合，互补滤波 α=0.95 |
| 电机 PWM | ✅ | TIM1, PA8/PA11, 10KHz |
| 编码器 | ✅ | 左 TIM2(PA0/PA1)，右 TIM4(PB6/PB7) |
| 串口 | ✅ | USART1 115200，printf 重定向 |
| ADC 电池电压 | ⚠️ | 分压比需校准，低压保护暂时绕过 |
| KEY2 启停 | ✅ | PA5 按键切换 Motor ON/OFF，30ms 去抖 |
| 平衡控制 | ⚠️ 调参中 | 串级 PID，直立 PD + 速度 PI，5ms 控制周期，电机方向已修正 |

---

## 引脚分配（最终确认版）

| 功能 | GPIO | 定时器 |
|------|------|--------|
| OLED RST | PB3 | — |
| OLED CS | PA7 | — |
| OLED RS/DC | PA15 | — |
| OLED SCK | PB5 | — |
| OLED MOSI | PB4 | — |
| MPU6050 SCL | PB8 | 软件 I2C |
| MPU6050 SDA | PB9 | 软件 I2C |
| MPU6050 AD0 | PA15 | 与 OLED RS 共用（仅上电采样） |
| MPU6050 INT | PA12 | EXTI15_10, 200Hz |
| 左电机 PWM | PA8 | TIM1_CH1 |
| 右电机 PWM | PA11 | TIM1_CH4 |
| 左电机 AIN1/2 | PB14/PB15 | TB6612 方向 |
| 右电机 BIN1/2 | PB13/PB12 | TB6612 方向 |
| 左编码器 A/B | PA0/PA1 | TIM2 |
| 右编码器 A/B | PB6/PB7 | TIM4 |
| 电池 ADC | PB0 | ADC1_IN8 |
| KEY2 | PA5 | EXTI9_5 |
| USART1 TX/RX | PA9/PA10 | — |

---

## 定时器分配

| 定时器 | 用途 |
|--------|------|
| TIM1 | 电机 PWM (CH1=PA8, CH4=PA11) |
| TIM2 | 左编码器 (CH1=PA0, CH2=PA1) |
| TIM3 | 5ms 定时器（仅内部，gtim.c） |
| TIM4 | 右编码器 (CH1=PB6, CH2=PB7) |

---

## 所有 Bug 修复记录（Session 1-3）

### Session 1 修复
- LED/KEY/EXTI/MOTOR/ENCODER/MPU6050/ADC/OLED 驱动全部重写
- TB6612 方向引脚添加
- readVolt 除零保护
- Vel_PI 抗积分饱和
- 字体精简（仅保留 16×16）

### Session 2 修复
- 编译配置修正（C99, MicroLIB, -Os）
- 寄存器版 ADC（替代 HAL_ADC，节省 2.7KB）
- DMA 空桩（节省 1.7KB）
- 移除未用 HAL 模块

### Session 3 修复（本次）

| # | 文件 | 问题 | 修复 |
|---|------|------|------|
| 1 | atk_ms6050.c:159 | MPU6050 INT 中断未使能 (0x00) | → 0x01 |
| 2 | atk_ms6050.c:158,167 | 采样率 50Hz | → 200Hz |
| 3 | encoder.c + gtim.c | 右编码器配错 TIM3(PA7) 与 OLED CS 冲突 | 右编码器→TIM4(PB6/PB7)，gtim→TIM3 |
| 4 | control.h/c | Velocity_Ki 类型 int（截断小数） | → float |
| 5 | gtim.c:Vel_PI | 电机禁用后积分不重置 | 检测 motor_enable 清零 |
| 6 | exti.c:KEY2 | 无按键消抖 | 30ms 时间窗口去抖 |
| 7 | exti.c:4 | `"math.h"` | → `<math.h>` |
| 8 | sys.c | HSE 起振失败→while(1) 死循环 | 添加 HSI+PLL 64MHz 回退 |
| 9 | main.c:18 | delay_init(72) 硬编码 | → SystemCoreClock/1000000 |
| 10 | oled.c/h | HAL_GPIO_Init 对 JTAG 引脚(PB3/PB4/PA15)配置失败 | 改为寄存器级 BSRR/BRR |
| 11 | main.c | oled_show_string 只写缓冲区，未推送到屏 | 主循环添加 oled_refresh_gram() |
| 12 | main.c | y 参数用 page 号(0,2,4,6)当像素用 | → 像素值(0,16,32,48) |
| 13 | exti.c | MPU6050 轴配错 (ax+gy→ay+gx) | 交换加速度计和陀螺仪轴 |
| 14 | control.c | 换轴后电机方向反了 | balance_pwm 取反（加负号） |
| 15 | control.c | MECHANICAL_ZERO 过时（旧轴 2.3°，新轴 0°） | → 0.0f |

---

## 已修改文件清单

```
Template/
├── User/main.c                    ← 主要修改：main 循环 + 桩函数
├── SYSTEM/sys/sys.c               ← HSE 回退 + SystemCoreClock 更新
├── HARDWARE/CONTROL/control.c     ← Velocity_Ki float + 机械零点
├── HARDWARE/CONTROL/control.h     ← Velocity_Ki extern float
├── HARDWARE/TIMER/gtim.c          ← TIM4→TIM3 + Vel_PI 清零
├── Drivers/BSP/OLED/oled.c        ← 寄存器 GPIO 初始化
├── Drivers/BSP/OLED/oled.h        ← 寄存器 BSRR/BRR 宏
├── Drivers/BSP/EXTI/exti.c        ← 轴组合 + 消抖 + 互补滤波 + 低压绕过
├── Drivers/BSP/ENCODER/encoder.c  ← TIM3→TIM4 右编码器
├── Drivers/BSP/ENCODER/encoder.h  ← 新增声明
├── Drivers/BSP/MPU6050/atk_ms6050.c ← INT 使能 + 200Hz
```

---

## PID 参数（最新值）

文件：[Template/HARDWARE/CONTROL/control.c](Template/HARDWARE/CONTROL/control.c)

```c
#define MECHANICAL_ZERO  0.0f    /* 换轴后直立 pitch≈0，无需偏移 */
float Balance_Kp = 150.0f;      /* 直立环比例 — 当前仍偏小，继续上调 */
float Balance_Kd = 0.8f;        /* 直立环微分 */
int   Velocity_Kp = 0;          /* 速度环比例 — 先保持 0，直立环调好再加 */
float Velocity_Ki = 0.0f;       /* 速度环积分 — 先保持 0 */
```

**注意：第 44 行 balance_pwm 已取反**（换轴后电机方向矫正）：

```c
balance_pwm = -(int)(Balance_Kp * (pitch - MECHANICAL_ZERO) + Balance_Kd * gyro_y);
```

### 当前现象：小角度时电机无力，倾角持续增大 → 需要 **增大 Balance_Kp**（先到 250，再调）

调参指南详见 [ee.md](ee.md)。

---

## 待办事项

| 优先级 | 任务 | 说明 |
|--------|------|------|
| 🔴 高 | PID 调参 | 按 ee.md 逐步调 Balance_Kp→Kd→Velocity_Kp→Ki |
| 🟡 中 | ADC 校准 | 万用表测电池电压 → 修正 adc.c 分压比 → 恢复低压保护 |
| 🟢 低 | 代码整理 | 清理多余的 debug 代码、桩函数优化等 |

---

## 调试技巧备忘

### OLED 确认存活
```c
// 在 main() 最开始，寄存器级 OLED 亮屏测试
oled_init();  // 现在使用寄存器 GPIO，不再依赖 HAL
oled_show_string(0, 0, "OK", 16);
oled_refresh_gram();  // 必须调用！否则不显示
```

### 串口观察
PA9/PA10，115200 波特率，输出格式：
```
P=2.3 G=0.1 V=12.3 S=15 E=1
```

### 关键注意
- **PB3、PB4、PA15 是 JTAG 引脚**，oled.c 内部已用 AFIO remap 释放
- **oled_refresh_gram() 必须调**，否则 OLED 不更新
- **MPU6050 轴组合**是 AY + GX（**不是**常见的 AX + GY）
- **balance_pwm 已取反**（第 44 行负号），换轴后必须这样才能正确回正
- **HSE 晶振可能不存在/损坏**，sys.c 有自动回退到 HSI+PLL
- **电机默认关闭**，需按 KEY2(PA5) 开启
- 浮点数常量必须写 `150.0f`，不能写 `150f`（Keil AC5 报错）
- 烧录后如果没反应 → **给芯片断电重启**（可能 HAL 状态异常）

---

## 快速恢复操作

如果换了电脑或重开 Keil：

1. 打开 `Template/final.uvprojx`
2. 确认编译宏：`USE_HAL_DRIVER,STM32F103xB`
3. 确认 Optimization: Level 1 (-O1)
4. 确认 C99 mode 和 MicroLIB
5. F7 编译 → F8 烧录
6. 小车断电重启 → 按 KEY2 开启电机

---

*Session 3 结束。下一步：按 ee.md 调参。*
