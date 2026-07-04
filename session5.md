# Session 5 — 平衡车 PID 调参与串口调试交接

---

## 当前状态

项目已经进入基于串口数据的 PID 调参阶段。串口乱码问题已通过强制使用 HSI+PLL 64MHz 修复；当前串口可以输出 CSV 调试数据。

| 项 | 当前值 |
|---|---|
| 工程目录 | `Template/` |
| 控制频率 | 200Hz，由 MPU6050 INT 触发 |
| 串口 | USART1 PA9/PA10, 115200, 8N1, 无流控 |
| 当前模式 | 电机已解除安全采样，`DEBUG_SENSOR_ONLY=0` |
| 当前目标 | 继续调直立环 PD，观察 CSV 数据调 PWM |

---

## 当前关键参数

文件：`Template/HARDWARE/CONTROL/control.c`

```c
#define MECHANICAL_ZERO  0.25f
#define SAFE_ANGLE_MAX   30.0f

float Balance_Kp = 260.0f;
float Balance_Kd = 2.00f;

int Velocity_Kp = 0;
float Velocity_Ki = 0.0f;

#define SPEED_DAMP_K         6.0f
#define SPEED_DAMP_DEADZONE  3

#define DEBUG_SENSOR_ONLY    0
#define DEBUG_PWM_LIMIT      800
```

文件：`Template/Drivers/BSP/MOTOR/motor.c`

```c
#define MOTOR_LEFT_DZ       100
#define MOTOR_RIGHT_DZ      100
#define MOTOR_DZ_START      100
#define MOTOR_SAFE_LIMIT    800
```

死区逻辑：

```c
if (pwm > 100)       pwm += 100;
else if (pwm < -100) pwm -= 100;
else                 pwm = 0;
```

---

## 串口调试输出格式

启动后先输出表头：

```text
DBG,t,p100,g10,sl,sr,ss,bp,sp,fl,fr,ml,mr,en,be
```

之后每 50ms 输出一行：

```text
D,t,p100,g10,sl,sr,ss,bp,sp,fl,fr,ml,mr,en,be
```

字段含义：

| 字段 | 含义 |
|---|---|
| `t` | `HAL_GetTick()`，毫秒 |
| `p100` | 倾角 `pitch * 100`，例如 `75` 表示 `0.75°` |
| `g10` | 角速度 `gyro_y * 10`，例如 `-25` 表示 `-2.5°/s` |
| `sl` | 左编码器 5ms 增量 |
| `sr` | 右编码器 5ms 增量 |
| `ss` | `sl + sr` |
| `bp` | 直立环 PD 输出，死区/限幅前 |
| `sp` | 速度阻尼输出 |
| `fl` | 左电机控制目标，进入 `motorMove()` 前 |
| `fr` | 右电机控制目标，进入 `motorMove()` 前 |
| `ml` | 左电机实际 PWM，经过死区补偿与电机限幅后 |
| `mr` | 右电机实际 PWM，经过死区补偿与电机限幅后 |
| `en` | `motor_enable`，按键启停状态 |
| `be` | `balance_enable`，平衡控制使能 |

注意：为了兼容 Keil MicroLIB，串口不输出浮点数，全部用整数缩放表示。

---

## 已完成的重要修改

### 1. 串口乱码修复

文件：`Template/SYSTEM/sys/sys.c`

强制使用 HSI+PLL 64MHz，不再依赖外部 HSE 晶振频率：

```c
HSI/2 * 16 = 64MHz
```

原因：之前如果 HSE 不是代码假设的 8MHz，USART 波特率会跑偏，导致乱码。

### 2. TIM3 定时自适应系统时钟

文件：`Template/HARDWARE/TIMER/gtim.c`

TIM3 prescaler 改为基于 `SystemCoreClock` 计算，保持 200Hz 控制周期。

### 3. 串口 CSV 数据

文件：`Template/User/main.c`

主循环每 50ms 输出 `D,...` 一行数据，便于根据真实动态调参。

### 4. 控制器调试变量

文件：`Template/HARDWARE/CONTROL/control.c/h`

新增：

```c
debug_balance_pwm
debug_speed_pwm
debug_final_left
debug_final_right
```

### 5. 安全采样模式

曾临时启用：

```c
#define DEBUG_SENSOR_ONLY 1
```

用于只输出数据不驱动电机，校验机械零点。当前已关闭：

```c
#define DEBUG_SENSOR_ONLY 0
```

---

## 调参过程中的关键发现

1. 完整实测死区 `左 285 / 右 280` 不能直接用偏移式补偿。  
   直接偏移会产生剧烈抖动，手按不住。

2. 硬阈值死区也不适合。  
   小角度时 `fl/fr` 有输出但 `ml/mr=0`，电机完全不动，等角度大了才猛冲。

3. 当前使用柔和死区：`START=100, DZ=100`。  
   小信号不动，中等以上信号加 100。

4. 左编码器疑似没有数据。  
   多组数据中 `sl` 基本一直为 `0`，右编码器 `sr` 有变化。后续启用正式速度环前必须检查左编码器 PA0/PA1、TIM2、接线和 AB 相。

5. 速度环 PI 暂时不要启用。  
   之前接入 `Velocity_Kp=15, Velocity_Ki=0.075` 后，出现平衡点被推到 `pitch=4~6°` 的现象。当前只保留非积分速度阻尼。

6. 机械零点曾多次随手扶姿态变化，不再继续纠结。  
   用户明确要求暂不纠结零点，先调 PID。当前用 `MECHANICAL_ZERO=0.25f`。

---

## 最近一次用户反馈

当前参数前一版：

```c
Balance_Kp = 260.0f;
Balance_Kd = 1.00f;
MOTOR_SAFE_LIMIT = 500;
```

用户给出的数据中：

```text
fl/fr 已经到 -800
但 ml/mr 被限制在 -500
```

判断：控制器已经想用更大力扶正，但实际电机被 `MOTOR_SAFE_LIMIT=500` 卡住。

因此已改为当前参数：

```c
Balance_Kp = 260.0f;
Balance_Kd = 2.00f;
MOTOR_SAFE_LIMIT = 800;
DEBUG_PWM_LIMIT = 800;
```

下一步：烧录当前版本，继续收集 `D,...` 数据。如果出现高频细碎抖动，优先把 `Balance_Kd` 降到 `1.5f`；如果仍然一倒就 `fl/fr/ml/mr` 全部顶到 `±800`，考虑继续提高限幅或提高 `Balance_Kp`。

---

## 下一轮建议流程

1. 烧录当前代码。
2. 串口设置 `115200, 8N1, 无流控`。
3. 按 KEY2 开电机。
4. 采集从松手前 1 秒到倒下后的 `D,...` 数据。
5. 优先根据以下关系判断：

| 现象 | 判断 |
|---|---|
| `ml/mr` 长时间顶到 `±800` 仍倒 | 力矩/限幅不够，或 Kp 偏低 |
| `p100` 变化不大但 `g10` 很大 | Kd 阻尼不够 |
| 静止附近 `ml/mr` 高频正负跳 | Kd 过高或死区补偿过激 |
| `sl` 一直为 0 | 左编码器仍异常，不能启用正式速度环 |

---

*Session 5 结束。下一轮从当前代码和本文件继续。*
