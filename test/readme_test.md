# Test 项目 — 最小烧录验证

## 功能

PA4 LED 以 1Hz 闪烁（开机快速闪 3 次确认启动，然后 500ms 交替亮灭）

## Keil MDK 配置步骤

### 1. 添加文件到工程

打开 `test.uvprojx`，在左侧 Project 窗口：

右键 `Target 1` → `Add Group` 创建分组：
- **Startup** → 右键 `Add Existing Files`，添加 `startup_stm32f103xb.s`
  （从 Template 项目的 `Template/Startup/` 目录复制过来，或用 Keil 自带的）

- **User** → 右键 `Add Existing Files`，添加 `test/main.c`

### 2. Options for Target 设置 (Alt+F7)

| 选项卡 | 配置项 | 值 |
|--------|--------|-----|
| **Target** | XTAL | `8.0` MHz |
| **Output** | Create HEX File | ✅ 勾选 |
| **C/C++** | Define | `STM32F103xB` |
| **C/C++** | Include Paths | 添加 CMSIS Include 路径（见下方） |
| **Debug** | Use | `ST-Link Debugger` |
| **Utilities** | Use Debug Driver | ✅ 勾选 |

### 3. CMSIS 包含路径

`C/C++` → `Include Paths` 添加（路径相对于 `test/` 目录）：

```
..\Template\Drivers\CMSIS\Include
..\Template\Drivers\CMSIS\Device\ST\STM32F1xx\Include
```

### 4. 编译 + 烧录

1. 按 **F7** 编译 → 确认 **0 Error, 0 Warning**
2. ST-Link 连接：SWDIO + SWCLK + GND + 3.3V
3. 按 **F8** 烧录 → 底部应显示 `Programming Done. Verify OK.`
4. 按小车复位键 → 观察 OLED

## 现象判断
观察oled屏，应该是一行一行亮起，然后熄灭


## 这个程序的特点

- **零 HAL 依赖** — 纯 CMSIS 寄存器操作，排除所有 HAL 配置问题
- **零头文件依赖** — 只包含 `<stm32f103xb.h>`（CMSIS 标准头文件）
- **约 200 字节** — 极简，MDK 评估版也能编译
- 使用 HSI 内部 8MHz 时钟，不依赖外部晶振
