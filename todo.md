# 平衡车项目实验任务清单

## 来源

`平衡车项目实验指导书.doc`（419页），STM32F103 平衡车项目完整实验教程。

---

## 阶段一：环境搭建

### 实验一：开发环境搭建
- [ ] 安装 MDK5（Keil IDE）
- [ ] 安装 STM32F4 器件支持包
- [ ] 安装仿真器驱动（推荐 DAP CMSIS-DAP Debugger）
- [ ] 安装 CH340 USB 虚拟串口驱动
- [ ] 验证串口识别正常（设备管理器显示 USB-SERIAL CH340）

### 实验二：HAL 库了解
- [ ] 理解 HAL 库（Hardware Abstraction Layer）概念
- [ ] 了解标准库 vs HAL 库 vs LL 库的区别
- [ ] 了解 CMSIS 标准架构（Device / Include 文件夹）
- [ ] 熟悉 STM32Cube 固件包目录结构（Drivers / Middlewares / Projects / Utilities）

### 实验三：新建 HAL 版本 MDK 工程
- [ ] 创建工程根目录文件夹结构（Drivers / Middlewares / Output / Projects / User / Startup）
- [ ] 从 STM32Cube_FW_F1_V1.8.0 拷贝所需文件
- [ ] 在 MDK 中创建新工程，选择 STM32F103C8T6
- [ ] 添加分组：启动文件、SYSTEM、User、STM32F1xx_HAL_Driver
- [ ] 配置 Target / Output / Listing / C/C++ / Debug / Utilities 选项
- [ ] C/C++ 宏定义：USE_HAL_DRIVER, STM32F103xB
- [ ] 配置头文件包含路径
- [ ] 编写 main.c，调用 HAL_Init() 和 HAL_IncTick()
- [ ] 编译验证（0 错误 0 警告）

---

## 阶段二：基础外设驱动

### 实验四：LED 控制
- [ ] 学习 GPIO 寄存器（MODER / OTYPER / OSPEEDR / PUPDR / IDR / ODR / BSRR）
- [ ] 了解 HAL GPIO API（HAL_GPIO_Init / HAL_GPIO_WritePin / HAL_GPIO_TogglePin）
- [ ] 使能 GPIOA 时钟：`HAL_RCC_GPIOA_CLK_ENABLE()`
- [ ] 编写 led.c / led.h（BSP 层），初始化 PA4 为推挽输出
- [ ] 在 main.c 中实现 LED0 以 500ms 间隔闪烁

### 实验五：按键输入实验
- [ ] 学习独立按键原理与按键抖动（5~10ms 消抖）
- [ ] 学习 GPIO 输入数据寄存器（IDR）
- [ ] 使用 HAL_GPIO_ReadPin() 读取 PA5 按键状态
- [ ] 实现按键消抖逻辑
- [ ] 用按键控制 LED 亮灭

### 实验六：外部中断实验
- [ ] 学习 NVIC（嵌套向量中断控制器）原理
- [ ] 学习 EXTI（外部中断/事件控制器）原理
- [ ] 使用 HAL 中断 API 配置 PA5 为外部中断输入
- [ ] 编写 EXTI 中断回调函数
- [ ] 在中断中实现按键控制 LED

---

## 阶段三：电机控制

### 实验七：电机 PWM 驱动实验
- [ ] 学习 PWM 脉宽调制原理（频率、占空比）
- [ ] 学习 STM32 定时器 PWM 输出模式（PWM 模式 1 / 模式 2）
- [ ] 配置定时器 14 产生 PWM 信号
- [ ] 编写 motor.c / motor.h 电机驱动文件
- [ ] 通过 PWM 占空比控制电机转速

### 实验八：电机测速实验
- [ ] 学习正交编码器原理（A/B 两相）
- [ ] 配置定时器为编码器模式
- [ ] 编写 encoder.c / encoder.h（ENCODER 文件夹）
- [ ] 实现 Read_Encoder() 读取编码器计数值
- [ ] 配置另一个定时器产生定时中断，周期性读取速度
- [ ] 根据编码器差值和时间间隔计算电机转速

### 实验九：电机速度闭环控制实验
- [ ] **自行补充 Vel_PI(int vel, int Target) 函数**（增量式 PI 控制器）
- [ ] **自行补充 PWM_Limit() 函数**（左右轮 PWM 限幅，最大 7200）
- [ ] 在定时中断中完成速度测量 + PI 控制计算
- [ ] 使用曲线调试工具调节 PI 参数
- [ ] 验证电机速度闭环控制效果

---

## 阶段四：通信与传感器

### 实验十：串口通信实验
- [ ] 学习 UART 串口通信协议（起始位 / 数据位 / 校验位 / 停止位）
- [ ] 了解 RS-232、TTL 电平、USB 转串口（CH340C）
- [ ] 配置 USART1（PA9=TX, PA10=RX）
- [ ] 使用 HAL UART API 实现数据收发
- [ ] 实现 printf 重定向到串口
- [ ] 电脑端使用 XCOM 等串口助手进行通信验证

### 实验十一：陀螺仪数据读取与 ADC 电压测量实验
- [ ] 了解 MPU6050（三轴加速度计 + 三轴陀螺仪，I²C 接口）
- [ ] 软件模拟 I²C 通信协议
- [ ] 编写 MPU6050 初始化代码（陀螺仪量程 ±2000°，加速度计量程 ±2g）
- [ ] 读取 MPU6050 原始数据并转换为角度值和角速度值
- [ ] 读取电池电压（ADC），低于 10V 时关闭平衡功能

### 实验十二：OLED 显示实验
- [ ] 了解 OLED 显示屏（SSD1306 控制器）
- [ ] 实现 4 线 SPI 接口通信
- [ ] 编写 OLED 初始化与基础显示函数
- [ ] 实现 ASCII 字符显示
- [ ] 在 OLED 上显示平衡车状态信息

---

## 阶段五：综合实验

### 实验十三：小车平衡控制综合实验（最终目标）
- [ ] 整合所有模块（MPU6050 姿态读取 + PID 平衡控制 + 电机 PWM 驱动 + 编码器测速 + OLED 显示）
- [ ] 实现直立平衡控制算法
- [ ] **验收标准 1**：小车能够直立稳定，稳定时轮向位移 < 5cm
- [ ] **验收标准 2**：具备抗干扰能力，给予 10° 以内摆幅干扰后能恢复平衡
- [ ] **验收标准 3**：电池电压 < 10V 时，平衡功能自动关闭（防止动力不足导致算法失效）
