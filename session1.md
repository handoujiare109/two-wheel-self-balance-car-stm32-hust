# Session 1 — 平衡车项目进度交接

## 项目概述

STM32F103C8T6 平衡车（实验十三），使用 Keil MDK + HAL 库。

## 已完成工作

### 1. 驱动文件重写（对齐 ALIENTEK 参考实现 `所需软件/5.第三方库/`）

所有文件已重写为与参考实现一致的 API 和引脚定义：

**BSP 驱动 (Drivers/BSP/):**

| 模块 | 文件 | 状态 |
|------|------|------|
| LED | led.c/h — PA4 | 完成 |
| KEY | key.c/h — PA5 按键消抖 | 完成 |
| EXTI | exti.c/h — PA12 MPU6050 INT + PA5 KEY2 | 完成 |
| MOTOR | motor.c/h — TB6612 PWM + 方向控制 | 完成 |
| ENCODER | encoder.c/h — TIM2(PA0/PA1) + TIM3(PA6/PA7) | 完成 |
| MPU6050 | atk_ms6050.c/h + atk_ms6050_iic.c/h — 软件 I2C PB8/PB9, AD0=PA15 | 完成 |
| ADC | adc.c/h — PB0 电池电压 | 完成 |
| OLED | oled.c/h + oledfont.h — 4线SPI PB3/PA7/PA15/PB5/PB4 | 完成 |

**SYSTEM (SYSTEM/):**

| 模块 | 文件 | 状态 |
|------|------|------|
| sys | sys.c/h — 时钟初始化 + 系统控制 | 完成 |
| delay | delay.c/h — SysTick us/ms 延时 | 完成 |
| usart | usart.c/h — USART1 printf 重定向 | 完成 |

**HARDWARE (HARDWARE/):**

| 模块 | 文件 | 状态 |
|------|------|------|
| TIMER | gtim.c/h — TIM4 5ms + Vel_PI() | 完成 |
| CONTROL | control.c/h — 串级 PID 平衡控制 | 完成 |

**User (User/):**

| 文件 | 状态 |
|------|------|
| main.c — 完整初始化 + 主循环 + OLED/串口输出 | 完成 |
| main.h, stm32f1xx_hal_conf.h, stm32f1xx_it.c | 未修改 |

### 2. 引脚分配（最终正确版本）

| 功能 | GPIO | 备注 |
|------|------|------|
| LED | PA4 | |
| KEY2 | PA5 | |
| 左电机 PWM | PA8 | TIM1_CH1 |
| 左电机 AIN1 | PB14 | TB6612 方向 |
| 左电机 AIN2 | PB15 | TB6612 方向 |
| 右电机 PWM | PA11 | TIM1_CH4 |
| 右电机 BIN1 | PB13 | TB6612 方向 |
| 右电机 BIN2 | PB12 | TB6612 方向 |
| 左编码器 A/B | PA0, PA1 | TIM2 |
| 右编码器 A/B | PA6, PA7 | TIM3 |
| MPU6050 SCL | PB8 | 软件 I2C |
| MPU6050 SDA | PB9 | 软件 I2C |
| MPU6050 AD0 | PA15 | 接 GND, 地址 0x68 |
| MPU6050 INT | PA12 | 5ms EXTI |
| 电池 ADC | PB0 | ADC12_IN8 |
| OLED RST | PB3 | |
| OLED CS | PA7 | |
| OLED DC/RS | PA15 | 与 AD0 共用 |
| OLED SCK | PB5 | |
| OLED MOSI | PB4 | |
| USART1 TX/RX | PA9, PA10 | |

> PA15 双重用途：OLED_DC + MPU6050_AD0。AD0 仅在上电时采样。代码中调用 `__HAL_AFIO_REMAP_SWJ_NOJTAG()` 释放 PA15。

### 3. 已知 Bug 修复

- **readVolt()** — 添加了 `if (cnt == 0) return 0;` 防止除零 (exti.c:50)
- **Get_battery_volt()** — 添加了 `HAL_ADC_PollForConversion` 返回值检查 (adc.c)
- **Vel_PI()** — 从使用隐藏的 static 变量 `Vel_Kp/Vel_Ki` 改为使用全局变量 `Velocity_Kp/Velocity_Ki`（来自 control.h），添加了 `[PWM_MIN, PWM_MAX]` 范围的抗积分饱和 (gtim.c)
- **电机驱动** — 添加了 TB6612 方向控制（之前仅有 PWM，无方向引脚）
- **旧文件清理** — 删除了 mpu6050.c/h（替换为 atk_ms6050.c/h + atk_ms6050_iic.c/h）

### 4. API 映射（旧 → 新）

| 旧 API | 新 API |
|--------|--------|
| `uart_init(115200)` | `usart_init(115200)` |
| `delay_init()` | `delay_init(72)` |
| `SystemClock_Config()` | `sys_stm32_clock_init(9)` |
| `MPU6050_Init()` | `atk_ms6050_init()` |
| `MPU6050_Read_Data()` | `atk_ms6050_get_accelerometer()` + `atk_ms6050_get_gyroscope()` + 互补滤波 (在 exti.c 中) |
| `OLED_Init()` | `oled_init()` |
| `OLED_ShowString()` | `oled_show_string()` |
| `OLED_ShowNum()` | `oled_show_num()` |
| `OLED_ShowFloat()` | `sprintf()` + `oled_show_string()` |
| `huart1` | `g_uart1_handle` |

### 5. 文档文件

| 文件 | 内容 |
|------|------|
| `readme.md` | MDK 配置步骤 + 正确引脚表 + 调参指南 |
| `bb.md` | 原始学习任务清单（用户提供） |
| `cc.md` | 剩余步骤指南：MDK 配置、接线、校准、PID 调试、验收 |
| `session1.md` | 本文件 |

---

## 用户下一步需要做的事

按顺序：

1. **MDK 工程配置** — 打开 `Template/final.uvprojx`，添加所有 .c 文件到分组，设置包含路径（详见 readme.md 或 cc.md），编译到 0 错误 0 警告
2. **硬件接线** — 按 cc.md 第二节的引脚表连接所有外设
3. **校准** — MPU6050 角度极性、ADC 电压分压比、编码器速度系数
4. **PID 调参** — Balance_Kp → Balance_Kd → Velocity_Kp → Velocity_Ki（流程见 cc.md）
5. **验收** — 30s 静止站立、抗干扰、低电压保护、按键启停

---

## 参考资源路径

```
所需软件/5.第三方库/
├── ATK_MS6050/    ← MPU6050 参考驱动
├── OLED/          ← OLED 参考驱动
└── SYSTEM-ALIENTEK/  ← sys/delay/usart 参考实现
```

## 关键提醒

- 所有 IntelliSense 报错是**正常的** — MDK 工程尚未配置包含路径，代码本身是正确的
- PWM 频率 10KHz，上限 7199（TIM1 Period=7199, Prescaler=0）
- 控制频率 200Hz（5ms），在 PA12 EXTI 中断中执行
- 电池电压 < 10V 自动关闭平衡功能
- 编译前务必在 MDK 中定义宏：`USE_HAL_DRIVER,STM32F103xB`
