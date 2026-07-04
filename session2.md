# Session 2 项目状态 - 平衡小车 Debug 准备

---

## 📋 项目概述

| 项 | 值 |
|---|---|
| 芯片 | STM32F103C8T6 (Cortex-M3, 72MHz, 128KB Flash) |
| 开发环境 | Keil MDK5.36 + ARM Compiler 5 (AC5) |
| HAL 库版本 | STM32F1xx HAL Driver |
| 当前状态 | ✅ 编译成功 (0 错误, 0 警告) |
| 代码总大小 | **32132 bytes** (Code=30300 + RO-data=1832) |
| MDK 评估版限制 | 32768 bytes |
| 剩余空间 | 636 bytes |

---

## ✅ 已完成的优化 (Session 1 汇总)

### 1. 编译错误修复
- ✅ 头文件路径配置修正
- ✅ Legacy/stm32_hal_legacy.h 路径修正
- ✅ C99 模式启用 (uC99=1)
- ✅ MicroLIB 启用 (useUlib=1)
- ✅ -Os 代码大小优化 (Optimization level 4)

### 2. 代码大小优化 (从 55KB → 32KB)

| 优化项 | 节省大小 |
|--------|---------|
| MicroLIB + -Os | ~5KB |
| 移除 OLED 12×12 和 24×24 字体 | ~450 bytes |
| HAL_ADC → 寄存器版重写 | ~740 bytes |
| HAL_DMA → 空桩函数替代 | ~1700 bytes |
| 移除 HAL 扩展模块 (tim_ex, adc_ex, rcc_ex, pwr) | ~2KB |

### 3. 关键桩函数 (main.c:73-105)

这些函数用于替代移除的 HAL 模块，满足链接器依赖：

```c
// TIM 扩展回调 (未使用高级定时器)
void HAL_TIMEx_BreakCallback(TIM_HandleTypeDef *htim);
void HAL_TIMEx_CommutCallback(TIM_HandleTypeDef *htim);
void TIMEx_DMACommutationCplt(DMA_HandleTypeDef *hdma);
void TIMEx_DMACommutationHalfCplt(DMA_HandleTypeDef *hdma);

// DMA 空桩 (不使用 DMA，节省 1.7KB)
HAL_StatusTypeDef HAL_DMA_Abort_IT(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma, ...);
HAL_StatusTypeDef HAL_DMA_Abort(DMA_HandleTypeDef *hdma);
uint32_t HAL_DMA_GetError(DMA_HandleTypeDef *hdma);

// RCC 扩展
uint32_t HAL_RCCEx_GetPeriphCLKFreq(uint32_t PeriphClk);  // 返回 36MHz
```

### 4. ADC 寄存器版重写 (adc.c)

移除了整个 `stm32f1xx_hal_adc.c` (2.7KB)，用直接寄存器操作替代，仅用于电池电压检测：
- 直接操作 RCC->APB2ENR, GPIOB->CRL, ADC1->CR1/CR2, SQR, SMPR
- 含软件校准流程
- 平衡控制完全不依赖 ADC，仅用于 OLED 显示电池电压

---

## 📁 项目文件结构

```
Template/
├── final.uvprojx          # Keil 工程文件 (关键配置)
├── final.axf              # 编译输出 (ELF 格式)
├── final.hex              # 编译输出 (Intel Hex，用于烧录)
│
├── User/
│   ├── main.c             # 主程序 + 桩函数
│   ├── stm32f1xx_hal_conf.h  # HAL 模块开关
│   ├── stm32f1xx_it.c     # 中断处理
│   └── system_stm32f1xx.c # 系统时钟配置
│
├── HARDWARE/
│   └── CONTROL/
│       └── control.c      # ⭐ PID 参数和控制逻辑 (当前打开)
│
├── Drivers/BSP/
│   ├── OLED/              # OLED 128×64 软件 SPI
│   │   ├── oled.c         # 显示驱动 (仅保留 16×16 字体)
│   │   └── oledfont.h     # 仅 16×16 ASCII 字符
│   ├── ADC/
│   │   ├── adc.c          # 寄存器版 ADC (无 HAL 依赖)
│   │   └── adc.h
│   ├── MOTOR/motor.c      # TB6612 电机驱动 + TIM1 PWM
│   ├── ENCODER/encoder.c  # TIM2/TIM3 编码器接口
│   ├── EXTI/exti.c        # MPU6050 INT 引脚 → 200Hz 控制环
│   └── TIMER/gtim.c       # TIM4 5ms 定时 + 速度计算
│
├── SYSTEM/
│   ├── sys/sys.c          # 系统时钟初始化 (72MHz HSE)
│   ├── delay/delay.c      # 毫秒/微秒延时
│   └── usart/usart.c      # USART1 115200 波特率 (printf 重定向)
│
└── Drivers/STM32F1xx_HAL_Driver/Src/
    ├── stm32f1xx_hal.c    # HAL 核心
    ├── stm32f1xx_hal_cortex.c  // Cortex-M3 NVIC/SysTick
    ├── stm32f1xx_hal_gpio.c    // GPIO
    ├── stm32f1xx_hal_rcc.c     // 时钟/RCC
    ├── stm32f1xx_hal_tim.c     // 定时器/PWM/编码器
    ├── stm32f1xx_hal_uart.c    // 串口
    ├── stm32f1xx_hal_exti.c    // 外部中断
    └── ⚠️ 以下已从工程移除:
        - stm32f1xx_hal_adc.c  (用寄存器版替代)
        - stm32f1xx_hal_dma.c  (用空桩替代)
        - stm32f1xx_hal_adc_ex.c, tim_ex.c, rcc_ex.c, pwr.c, spi.c 等
```

---

## 🎛️ PID 参数位置 (当前调试目标)

**文件：** `Template/HARDWARE/CONTROL/control.c`

```c
/* 直立环 PD 参数 (当前值) */
float Balance_Kp = 200.0f;   // 比例系数
float Balance_Kd = 0.5f;     // 微分系数

/* 速度环 PI 参数 (当前值) */
int Velocity_Kp = 50;        // 比例系数
int Velocity_Ki = 0;         // 积分系数
```

### 控制时序
- **200Hz (5ms)** 控制周期，由 MPU6050 INT 引脚触发 EXTI 中断
- **平衡控制函数：** `Balance_Control(void)` (control.c:29)
- **速度计算：** `gtim.c` 中 TIM4 5ms 中断读取编码器

---

## 🔧 硬件连接备忘

| 功能 | 引脚 | 备注 |
|------|------|------|
| MPU6050 INT | PA12 | 200Hz 中断触发控制环 |
| KEY2 | PA5 | 电机使能/禁用切换 |
| 电机 PWM | PA8 (TIM1_CH1), PA11 (TIM1_CH4) | 10KHz |
| 电机方向 | PB12, PB13, PB14, PB15 | TB6612 AIN1/AIN2/BIN1/BIN2 |
| 左编码器 | PA0, PA1 (TIM2) | 正交编码模式 |
| 右编码器 | PB6, PB7 (TIM3) | 正交编码模式 |
| ADC 电池电压 | PB0 (ADC1_IN8) | 电阻分压 10K+33K |
| OLED | PB1~PB5 (软件 SPI) | 128×64 |
| USART1 | PA9 (TX), PA10 (RX) | 115200 波特率 |

---

## 📊 串口调试输出

波特率 115200，每 50ms 输出一行：
```
P=12.3 G=-0.5 V=7.4 S=0 E=1
│     │       │       │    └─ 平衡使能 (1=开启)
│     │       │       └─────── 左右轮编码器速度和
│     │       └─────────────── 电池电压 (V)
│     └─────────────────────── 陀螺仪角速度 (°/s)
└───────────────────────────── 车身倾角 (°)
```

---

## 🚀 烧录方式

**Keil 内一键烧录：**
1. ST-Link 连接 SWDIO + SWCLK + GND + 3.3V
2. Keil 中按 **F8** 或点击 Load 按钮
3. 或调试模式按 **Ctrl+F5** 可在线调试断点

---

## ❗ 已知注意事项

1. **代码空间紧张：** 仅剩 636 字节，添加新功能前需进一步优化
2. **HAL 头文件依赖：** `stm32f1xx_hal_conf.h` 中模块开关需与实际编译的 .c 文件对应
3. **DMA 空桩风险：** 不要调用任何 HAL_DMA_* 函数，空桩无实际功能
4. **ADC 仅用于显示：** 平衡算法不依赖 ADC，电池电压异常不影响站立
5. **字体已裁剪：** OLED 仅支持 16×16 ASCII，不要调用 12×12 或 24×24 显示函数

---

## 🎯 Debug 会话 2 预期任务

1. **烧录验证：** 确认代码下载到小车后运行正常
2. **串口数据检查：** 观察倾角 P、角速度 G、速度 S 是否合理
3. **机械零点校准：** 确定小车直立时的倾角基准值
4. **直立环 PD 调参：** Balance_Kp → Balance_Kd
5. **速度环 PI 调参：** Velocity_Kp → Velocity_Ki
6. **问题定位：** 如抖动、跑偏、无法站立等现象分析

---

*此文件用于 Session 2 快速上手项目状态，debug 时可参考*
