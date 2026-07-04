# 实验十三：平衡车 — 剩余步骤指南

---

## 当前状态

所有源代码已编写完毕（32 个文件），编译前的准备工作已完成。以下是你还需要亲自完成的步骤。

---

## 一、MDK 工程配置（必须手动完成）

### 1.1 打开工程
双击 `Template/final.uvprojx` 用 Keil MDK 打开。

### 1.2 确认芯片型号
`Project → Options for Target → Device` → 选择 `STM32F103C8T6`

### 1.3 添加源文件到分组

MDK 左侧 Project 窗口，右键 `Target 1` → `Add Group`，创建以下分组，然后右键每个分组 `Add Existing Files`：

| 分组 | 添加的文件 |
|------|-----------|
| **Startup** | `Startup/startup_stm32f103xb.s` |
| **SYSTEM** | `SYSTEM/sys/sys.c`、`SYSTEM/delay/delay.c`、`SYSTEM/usart/usart.c` |
| **User** | `User/main.c`、`User/stm32f1xx_it.c` |
| **HARDWARE** | `HARDWARE/TIMER/gtim.c`、`HARDWARE/CONTROL/control.c` |
| **BSP** | `Drivers/BSP/LED/led.c`、`Drivers/BSP/KEY/key.c`、`Drivers/BSP/EXTI/exti.c`、`Drivers/BSP/MOTOR/motor.c`、`Drivers/BSP/ENCODER/encoder.c`、`Drivers/BSP/MPU6050/atk_ms6050.c`、`Drivers/BSP/MPU6050/atk_ms6050_iic.c`、`Drivers/BSP/ADC/adc.c`、`Drivers/BSP/OLED/oled.c` |
| **STM32F1xx_HAL_Driver** | `Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c`、`stm32f1xx_hal_rcc.c`、`stm32f1xx_hal_gpio.c`、`stm32f1xx_hal_cortex.c`、`stm32f1xx_hal_tim.c`、`stm32f1xx_hal_uart.c`、`stm32f1xx_hal_exti.c`、`stm32f1xx_hal_dma.c`、`stm32f1xx_hal_flash.c`、`stm32f1xx_hal_pwr.c`、`stm32f1xx_hal_adc.c`、`stm32f1xx_hal_spi.c` |
| **CMSIS** | `Drivers/CMSIS/Device/ST/STM32F1xx/Source/system_stm32f1xx.c` |

### 1.4 配置头文件包含路径

`Project → Options for Target → C/C++` → 点击 `Include Paths` 右边的 `...`，添加以下路径：

```
.\Drivers\CMSIS\Include
.\Drivers\CMSIS\Device\ST\STM32F1xx\Include
.\Drivers\STM32F1xx_HAL_Driver\Inc
.\Drivers\BSP\LED
.\Drivers\BSP\KEY
.\Drivers\BSP\EXTI
.\Drivers\BSP\MOTOR
.\Drivers\BSP\ENCODER
.\Drivers\BSP\MPU6050
.\Drivers\BSP\ADC
.\Drivers\BSP\OLED
.\SYSTEM\sys
.\SYSTEM\delay
.\SYSTEM\usart
.\HARDWARE\TIMER
.\HARDWARE\CONTROL
.\User
```

### 1.5 其他 Options for Target 设置

| 选项卡 | 配置项 | 值 |
|--------|--------|-----|
| **Target** | XTAL | `8.0` MHz |
| **Output** | Create HEX File | 勾选 |
| **Output** | Select Folder for Objects | `Output/` |
| **Listing** | Select Folder for Listing | `Output/` |
| **C/C++** | Define | `USE_HAL_DRIVER,STM32F103xB` |
| **C/C++** | Optimization | Level 1 (-O1) |

### 1.6 编译

按 `F7` 编译，确保 **0 错误 0 警告**。

---

## 二、硬件接线检查

编译通过后，按照以下引脚表连接所有外设。**接线前务必断开电源。**

| 功能 | GPIO | 接法说明 |
|------|------|---------|
| LED | PA4 | 板载 LED 或外接 LED（串 1K 电阻到 GND） |
| 按键 KEY2 | PA5 | 一端接 PA5，另一端接 GND（内部上拉） |
| 左电机 PWM | PA8 | 接 TB6612 PWMA |
| 左电机 AIN1 | PB14 | 接 TB6612 AIN1 |
| 左电机 AIN2 | PB15 | 接 TB6612 AIN2 |
| 右电机 PWM | PA11 | 接 TB6612 PWMB |
| 右电机 BIN1 | PB13 | 接 TB6612 BIN1 |
| 右电机 BIN2 | PB12 | 接 TB6612 BIN2 |
| 左编码器 A | PA0 | TIM2_CH1 |
| 左编码器 B | PA1 | TIM2_CH2 |
| 右编码器 A | PA6 | TIM3_CH1 |
| 右编码器 B | PA7 | TIM3_CH2 |
| MPU6050 SCL | PB8 | 软件 I2C 时钟线 |
| MPU6050 SDA | PB9 | 软件 I2C 数据线 |
| MPU6050 AD0 | PA15 | 接 GND（地址 0x68） |
| MPU6050 INT | PA12 | 5ms 外部中断 |
| 电池电压 ADC | PB0 | 电池分压输出（分压比 10:43 → 12V→2.79V） |
| OLED RST | PB3 | 4 线 SPI OLED 复位 |
| OLED CS | PA7 | 4 线 SPI OLED 片选 |
| OLED DC/RS | PA15 | 4 线 SPI OLED 数据/命令（与 AD0 共用） |
| OLED SCK | PB5 | 4 线 SPI OLED 时钟 |
| OLED MOSI | PB4 | 4 线 SPI OLED 数据 |
| USART1 TX | PA9 | 串口工具 RX（波特率 115200） |
| USART1 RX | PA10 | 串口工具 TX |

> **关键提醒**: PA15 同时接 OLED_DC 和 MPU6050_AD0。MPU6050 只在复位时采样 AD0（低电平 → 地址 0x68），初始化后可安全复用。代码中已调用 `__HAL_AFIO_REMAP_SWJ_NOJTAG()` 释放 PA15。

---

## 三、真机调试（核心步骤）

### 3.1 确认 MPU6050 安装方向

将小车竖直放置，用串口工具（115200 波特率）观察输出：

```
printf("P=%.1f G=%.1f V=%.1f S=%d E=%d\r\n", pitch, gyro_y, battery_voltage, ...);
```

- 小车**前倾**时 `pitch` 应为**正值**
- 如果符号反了，在 [exti.c](Template/Drivers/BSP/EXTI/exti.c) 第 68 行调整：
  - `pitch` 取反：`pitch = -comp_angle;`
  - 或 `gyro_y` 取反：`gyro_y = -(float)gy / 16.4f;`

### 3.2 校准 ADC 电池电压

1. 用万用表实测电池实际电压（如 11.5V）
2. 对照串口输出的 `battery_voltage` 值
3. 如有偏差，在 [adc.c](Template/Drivers/BSP/ADC/adc.c) 第 49 行修改分压比：
   ```c
   float volt = (float)adc_val * 3.3f * 43.0f / 4096.0f / 10.0f;
   //                                  ^^^^              ^^^^
   //                              修改 R1+R2 比      修改 R1 值
   ```

### 3.3 编码器速度系数校准

[gtim.c](Template/HARDWARE/TIMER/gtim.c) 中当前速度计算：
```c
speed_left  = left  - enc_left_prev;   // 5ms 内的脉冲差值
speed_right = right - enc_right_prev;
```

如需转换为实际速度（mm/s），根据编码器线数计算系数：
- 常见电机编码器：11 线 → 减速比 30:1 → 轮子转一圈 = 11×30 = 330 个脉冲
- 轮子周长 = π × D = 3.14 × 65mm ≈ 204mm
- 每个脉冲对应距离 = 204 / 330 ≈ 0.62mm
- 5ms 内测得 N 个脉冲 → 速度 = N × 0.62mm / 0.005s = N × 124 mm/s

---

## 四、PID 参数调试（在真机上完成）

**调试顺序：先内环（直立 PD），后外环（速度 PI）。** 所有参数在 [control.c](Template/HARDWARE/CONTROL/control.c) 中修改。

### 4.1 直立环 — Balance_Kp（比例）

1. 设 `Balance_Kp = 0`，`Balance_Kd = 0`，`Velocity_Kp = 0`，`Velocity_Ki = 0`
2. 用手扶住小车保持竖直，逐步增大 `Balance_Kp`（每次 +20）
3. 感受到小车有明显的"回复力"后，放手让小车自由摆动
4. 继续增大直到小车出现**低频抖动**（来回摆动越来越快）
5. 记录该值，**乘以 0.6** 作为最终 `Balance_Kp`

### 4.2 直立环 — Balance_Kd（微分）

1. 在 4.1 的基础上，逐步增大 `Balance_Kd`（每次 +0.1）
2. 微分项用于**抑制低频抖动**，让小车更"硬"
3. 继续增大直到出现**高频抖动**（嗡嗡声、电机微振）
4. 记录该值，**乘以 0.6** 作为最终 `Balance_Kd`

### 4.3 速度环 — Velocity_Kp

1. 设 `Velocity_Kp = 0`，`Velocity_Ki = 0`
2. 此时小车应该能**短暂站立**（几秒），但会有来回漂移
3. 逐步增大 `Velocity_Kp`（每次 +5）
4. 观察小车：目标是在漂移时能"拉回来"
5. 过大时小车会出现**前后摆动**（过度纠正）
6. 找到临界值，**乘以 0.6** 作为最终 `Velocity_Kp`

### 4.4 速度环 — Velocity_Ki

通常设为 `Velocity_Ki = Velocity_Kp / 200`。

### 4.5 典型参考值

| 参数 | 初始值 | 典型范围 |
|------|--------|---------|
| Balance_Kp | 200 | 100 ~ 500 |
| Balance_Kd | 0.5 | 0.1 ~ 2.0 |
| Velocity_Kp | 50 | 20 ~ 200 |
| Velocity_Ki | 0 | Velocity_Kp / 200 |

---

## 五、验收标准

调试完成后，逐项验证：

1. **直立稳定**：小车能原地站立 **30 秒以上**，无明显漂移
2. **抗干扰**：用手轻推小车，小车能**自动恢复直立**
3. **位移范围**：30 秒内位移 **< 5cm**
4. **低电压保护**：电池电压 < 10V 时，小车的电机**自动停止**（平衡功能关闭）
5. **按键启停**：按 KEY2（PA5）能**切换电机启停**

---

## 六、使用上位机调试工具（可选）

如果实验指导书提供了 `curveShow` 或类似的上位机曲线工具：

1. 确认串口波特率匹配（115200）
2. 在 [main.c](Template/User/main.c) 的 `while(1)` 循环中，`printf` 已经每 50ms 输出一次姿态数据
3. 上位机可以实时绘制 pitch、gyro_y、速度、电压曲线
4. 用曲线观察：
   - `pitch` 曲线是否收敛到 0
   - `gyro_y` 是否在 0 附近小幅波动
   - `battery_voltage` 是否稳定

---

## 七、常见问题排查

| 现象 | 可能原因 | 解决方法 |
|------|---------|---------|
| MPU6050 ID 检测失败 | I2C 接线错误或 AD0 电平不对 | 检查 PB8/PB9 和 PA15 接线 |
| OLED 不显示 | SPI 接线错误 | 检查 PB3/PA7/PA15/PB5/PB4 |
| 电机不转 | TB6612 供电或方向引脚 | 检查 12V 供电、PB12-15 方向引脚 |
| 小车朝一个方向加速 | MPU6050 角度极性反了 | 按 3.1 节取反 pitch |
| 小车剧烈抖动 | Balance_Kp 或 Balance_Kd 过大 | 按 4.1/4.2 节减小参数 |
| 小车倒下回不来 | Balance_Kp 过小 | 增大 Balance_Kp |
| 电池电压显示为 0 或异常 | 分压公式与实际电阻不符 | 按 3.2 节校准 |
| 编码器速度恒为 0 | 编码器接线或定时器配置 | 检查 PA0/PA1/PA6/PA7 接线 |

---

## 八、参考文件对照

如果在调试过程中遇到问题，可以对照参考实现：

| 模块 | 参考路径 |
|------|---------|
| OLED 驱动 | `所需软件/5.第三方库/OLED/` |
| MPU6050 驱动 | `所需软件/5.第三方库/ATK_MS6050/` |
| SYSTEM 模块 | `所需软件/5.第三方库/SYSTEM-ALIENTEK/` |
| 完整引脚表 | `Template/readme.md` |
| 学习任务清单 | `bb.md` |
