# 实验十三：小车平衡控制综合实验 — 学习路径与任务清单

---

## 一、你需要学习的知识点

### 1. PID 控制理论（核心）

| 知识点 | 说明 |
|--------|------|
| **直立环（PD 控制器）** | Balance_Kp（比例）：根据倾角大小输出 PWM；Balance_Kd（微分）：根据角速度抑制震荡 |
| **速度环（PI 控制器）** | Velocity_Kp：根据速度偏差输出 PWM；Velocity_Ki：累积误差消除稳态偏差 |
| **串级 PID** | 直立环为内环（快，5ms 一次），速度环为外环（慢），内环输出叠加外环输出 |
| **增量式 PI** | `Vel_PI(int vel, int Target)` 函数，本次用增量式，公式：`ΔU = Kp*(e(k)-e(k-1)) + Ki*e(k)` |
| **调参方法** | 先内环后外环：先调 Balance_Kp 到低频抖动 → 再调 Balance_Kd 到高频抖动 → 两者 ×0.6 → 最后调速度环 |

### 2. MPU6050 姿态传感器
- I2C 通信协议（软件模拟 I2C）
- 三轴加速度计 + 三轴陀螺仪数据读取
- 倾角计算方法（加速度计反正切 + 陀螺仪积分）
- 互补滤波 / 卡尔曼滤波（融合加速度计和陀螺仪数据）

### 3. 正交编码器测速
- A/B 两相正交编码器原理
- STM32 定时器编码器模式配置
- 速度计算：读取编码器差值 ÷ 时间间隔（5ms）

### 4. 电机 PWM 驱动
- PWM 脉宽调制（频率、占空比）
- STM32 定时器 PWM 输出（PWM 模式 1/2）
- 死区控制和 H 桥驱动（TB6612 等电机驱动芯片）

### 5. ADC 电压测量
- 12 位逐次逼近型 ADC
- 电阻分压原理（电源 12V → ADC 3.3V 满量程）
- 平均值滤波（readVolt 函数）

### 6. 外部中断与定时器
- EXTI 外部中断配置（PA12 用于 MPU6050 5ms 中断）
- 定时器周期中断（用于速度测量和控制计算）
- NVIC 中断优先级

### 7. OLED 显示（SPI 接口）
- SSD1306 控制器，4 线 SPI
- 显存操作（GRAM 128×64）
- ASCII 字符显示

### 8. UART 串口通信
- USART1（PA9=TX, PA10=RX）
- printf 重定向到串口
- 串口助手/曲线调试工具使用

---

## 二、你需要完成的任务

### 第一步：搭建完整工程文件结构

在已有 Template 工程基础上，新建 BSP 和 SYSTEM 文件夹：

```
Template/
├── Drivers/
│   ├── CMSIS/          ← 已有（实验三完成）
│   ├── STM32F1xx_HAL_Driver/  ← 已有
│   └── BSP/            ← 新建，存放各外设驱动
│       ├── LED/        → led.c, led.h
│       ├── KEY/        → key.c, key.h
│       ├── EXTI/       → exti.c, exti.h
│       ├── MOTOR/      → motor.c, motor.h
│       ├── ENCODER/    → encoder.c, encoder.h
│       ├── MPU6050/    → mpu6050.c, mpu6050.h
│       ├── ADC/        → adc.c, adc.h
│       └── OLED/       → oled.c, oled.h, oledfont.h
├── SYSTEM/             ← 新建，系统级工具
│   ├── sys/            → sys.c, sys.h
│   ├── delay/          → delay.c, delay.h
│   └── usart/          → usart.c, usart.h
├── User/               ← 已有，需补充
│   ├── main.c         ← 重写
│   ├── stm32f1xx_hal_conf.h
│   └── stm32f1xx_it.c
└── Startup/            ← 已有
```

### 第二步：编写/补充各外设驱动文件

| 文件 | 需要你编写/补充的函数 | 难度 |
|------|----------------------|------|
| **exti.c** | `EXTIX_Init()` — 配置 PA12 为 5ms 外部中断入口；补充 `readVolt(cnt)` 平均值滤波（有 bug 需修复） | 高 |
| **motor.c** | `motorMove(speedL, speedR)` — 设置左右轮 PWM 占空比；`turn_Off()` — 停止电机；`PWM_Limit()` — PWM 限幅 7200 | 中 |
| **encoder.c** | `Read_Encoder(timer)` — 读取编码器当前计数值 | 低 |
| **gtim.c** | `Vel_PI(vel, Target)` — 增量式 PI 速度控制器 | 高 |
| **adc.c** | `Get_battery_volt()` — 单次 ADC 电压测量（需理解分压电路换算） | 中 |
| **mpu6050.c** | `MPU6050_Init()` — 初始化传感器；`MPU6050_Read_Data()` — 读取角度/角速度 | 高 |
| **oled.c** | OLED 初始化、清屏、显示字符串/数字 | 中 |
| **usart.c** | printf 重定向、串口收发 | 低 |
| **main.c** | 外设初始化 + 主循环（每 50ms 发送曲线数据 + OLED 刷新 + 按键处理） | 中 |
| **control.c** | 直立环 PD + 速度环 PI 串级控制，在 5ms EXTI 中断中调用 | 高 |

### 第三步：在 MDK 中配置工程
- 添加所有 .c 文件到对应分组
- 添加头文件包含路径
- 编译通过（0 错误 0 警告）

### 第四步：PID 参数调试（必须在真机上完成）
- 调直立环 Balance_Kp / Balance_Kd
- 调速度环 Velocity_Kp / Velocity_Ki
- 验证验收标准：直立稳定、抗干扰、低电压保护

---

## 三、我能帮你做的 vs 你必须自己做的

### 我可以帮你做的（写代码）

| 任务 | 说明 |
|------|------|
| 所有外设驱动文件 | 基于 STM32 HAL 库标准模式编写 led.c/h, key.c/h, motor.c/h, encoder.c/h, mpu6050.c/h, oled.c/h, oledfont.h, adc.c/h, usart.c/h, sys.c/h, delay.c/h |
| exti.c / exti.h | 编写 5ms 外部中断框架，包含 MPU6050 读数 + ADC 电压 + 控制逻辑调度 |
| control.c / control.h | 编写串级 PID 平衡控制算法（直立 PD + 速度 PI） |
| main.c | 编写完整的初始化 + 主循环逻辑 |
| 目录结构 + 工程配置文档 | 创建所有文件夹，输出 MDK 配置步骤 |

### 你必须自己做的

| 任务 | 原因 |
|------|------|
| 在 MDK 中创建工程、添加文件、配置选项 | GUI 操作，必须手动完成 |
| **PID 参数调试** | 需要在真机上观察小车抖动/回摆情况，逐步调整 Balance_Kp、Balance_Kd、Velocity_Kp、Velocity_Ki |
| 根据实际电路图校准 ADC 分压换算公式 | 物理电路参数我不知道 |
| 根据实际 MPU6050 安装方向调整角度极性 | 传感器焊接方向影响正负号 |
| 根据编码器线数计算速度转换系数 | 电机和编码器规格需要实物确认 |
| 验证验收标准 | 需要真机观察（位移 < 5cm、抗干扰、低电压保护） |
| **修复 readVolt() 的 bug** | 指导书中明确说这个函数有 bug，需要理解代码逻辑后修复 |
| 补充 Vel_PI() 函数体 | 指导书要求自行补充增量式 PI 公式 |
| 补充 PWM_Limit() 函数体 | 指导书要求自行补充限幅逻辑 |
| 补充 motorMove() / turn_Off() 函数体 | 指导书要求自行编写 |
| 曲线调试工具（curveShow）对接 | 需要实际连接上位机观察控制效果 |

---

## 四、建议执行顺序

```
1. 我先帮你写完所有驱动文件
2. 你在 MDK 中配置工程、编译
3. 你在真机上调试 PID 参数
4. 用上位机曲线工具观察控制效果
5. 完成验收
```

---

## 五、关键提醒

1. **PWM 上限 7200** 代表 100% 占空比，所有 PWM 输出必须经过 `PWM_Limit()` 限幅
2. **控制频率 200Hz**（5ms 一次），在 EXTI 中断中执行，不要在 main 循环里做控制
3. **电池电压 < 10V 时关闭平衡功能**，否则低电压下算法会失效导致小车失控
4. **先内环后外环**：直立环 PD 调好 → 再加速度环 PI
5. **编码器左右相加** 作为速度反馈，目标速度一般为 0（站立不动）
