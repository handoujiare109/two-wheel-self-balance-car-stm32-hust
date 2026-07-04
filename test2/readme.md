# test2 — 电机死区 + 极性测试

## 目的

1. **极性验证** — 确认 TB6612 方向引脚与电机转向的对应关系
2. **死区测量** — 找到电机刚好开始稳定转动的最小 PWM 值

## 接线

| test2 主程序 | GPIO | 说明 |
|-------------|------|------|
| 左电机 PWM | PA8 | TIM1_CH1 |
| 右电机 PWM | PA11 | TIM1_CH4 |
| 左电机 AIN1 | PB14 | TB6612 方向 |
| 左电机 AIN2 | PB15 | TB6612 方向 |
| 右电机 BIN1 | PB13 | TB6612 方向 |
| 右电机 BIN2 | PB12 | TB6612 方向 |
| LED | PA4 | 状态指示 |
| KEY2 | PA5 | 按键步进 |
| USART1 TX | PA9 | 115200 调试输出 |

**与主项目接线完全一致**，无需重新接线。

## Keil MDK 配置

### 1. 打开工程
双击 `test2.uvprojx`

### 2. 确认设置
| 选项卡 | 配置项 | 值 |
|--------|--------|-----|
| C/C++ | Define | `STM32F103xB` |
| C/C++ | Include Paths | `..\Template\Drivers\CMSIS\Include` `..\Template\Drivers\CMSIS\Device\ST\STM32F1xx\Include` |
| Debug | Use | `ST-Link Debugger` |

### 3. 编译 + 烧录
F7 → 确保 0 Error 0 Warning → F8 烧录

## 使用步骤

### 启动确认
烧录后按复位键，LED 快闪 3 次表示程序正常启动。

### Test 1 — 极性验证
1. 串口助手 115200 连接 PA9
2. 按 KEY2 开始
3. 电机**正转** 2 秒（PWM=+500, 向前），观察轮子方向
4. 电机**反转** 2 秒（PWM=-500, 向后），观察轮子方向
5. 重复 3 轮

**判断**：
- 正转时轮子**向前**转（推车前进）→ 极性正确
- 正转时轮子**向后**转 → 极性反了，需在 control.c 中去掉 `-(` 负号

### Test 2 — 死区扫描
1. 按 KEY2 进入 Test 2
2. PWM 从 60 开始，每次增加约 40
3. 每个值运行 2 秒，观察电机是否转动
4. 按 KEY2 跳到下一个值
5. 找到电机**刚好开始稳定转动**的最小 PWM 值

**结果换算**：
```
主项目死区 = 测试值 × 1.125  (= 测试值 × 7200/6400)
```

例如：测试值 250 → 主项目死区约 281

## 串口输出示例

```
========================================
  Motor Dead Zone & Polarity Test
  SYSCLK = 64MHz, PWM = 10KHz
========================================

[Test 1] Polarity Check
  PWM+ : wheel should turn FORWARD
  PWM- : wheel should turn BACKWARD
  Press KEY2 to start...

--- Round 1 / 3 ---
  FORWARD  (PWM=+500) ... observe wheel direction, then...
  BACKWARD (PWM=-500) ... observe wheel direction.

[Test 2] Dead Zone Scan
  [1/17] PWM=60   (main equiv ~68)
  [2/17] PWM=80   (main equiv ~90)
  [3/17] PWM=100  (main equiv ~113)
  ...
```

## 注意事项

- 测试期间电机空转即可，**不需要装到车上**
- PWM 范围 0-6399（10KHz，64MHz 时钟），与主项目的 0-7199（72MHz）略有差异
- 串口输出中 "main equiv" 列给出了换算到主项目的等效值
- 测试完成后按 KEY2 软件复位，重新运行