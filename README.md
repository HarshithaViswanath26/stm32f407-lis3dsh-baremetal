# STM32F407 Baremetal Driver Library
### LIS3DSH Accelerometer — SPI, USART, GPIO, EXTI, NVIC

Baremetal peripheral driver library for the STM32F407VG (STM32F4-Discovery board), written from scratch without HAL or CubeMX. Implements SPI communication with the LIS3DSH MEMS accelerometer, interrupt-driven data acquisition via EXTI, tilt detection via XYZ axis thresholding, and USART serial output.

---

## Hardware

| Component | Detail |
|---|---|
| MCU | STM32F407VGT6 |
| Board | STM32F4-Discovery |
| Sensor | LIS3DSH (WHO_AM_I = 0x3F) |
| Clock | 16MHz HSI (no PLL) |

---

## Pin Mapping

| Pin | Function | Peripheral |
|---|---|---|
| PA5 | SPI1 SCK | LIS3DSH SCL |
| PA6 | SPI1 MISO | LIS3DSH SDO |
| PA7 | SPI1 MOSI | LIS3DSH SDA/SDI |
| PE3 | GPIO Output (CS) | LIS3DSH CS |
| PE0 | GPIO Input (EXTI0) | LIS3DSH INT1 |
| PA2 | USART2 TX | Serial output |
| PD12 | GPIO Output | Green LED |
| PD13 | GPIO Output | Orange LED |
| PD14 | GPIO Output | Red LED |
| PD15 | GPIO Output | Blue LED |

---

## Project Structure

```
stm32f407-lis3dsh-baremetal/
├── Src/
│   └── lis3dsh_test.c       — main application, EXTI0 handler
├── Drivers/
│   ├── Inc/
│   │   ├── stm32f4xx.h      — MCU register definitions, base addresses
│   │   ├── gpio.h
│   │   ├── spi.h
│   │   ├── usart.h
│   │   ├── exti.h
│   │   ├── rcc.h
│   │   └── lis3dsh.h        — sensor register map, API declarations
│   └── Src/
│       ├── gpio.c           — GPIO driver
│       ├── spi.c            — SPI driver
│       ├── usart.c          — USART driver
│       ├── exti.c           — EXTI/NVIC driver
│       ├── rcc.c            — RCC clock driver
│       └── lis3dsh.c        — LIS3DSH sensor driver
└── Startup/
    └── startup_stm32f407vgtx.s
```

---

## Driver Overview

### GPIO (`gpio.c`)
- `GPIO_Init()` — configures mode, speed, output type, pull-up/down, alternate function
- `GPIO_Led_ON()` / `GPIO_Led_OFF()` — drives LED pins

### SPI (`spi.c`)
- `SPI_Init()` — configures master mode, full duplex, CPOL/CPHA, prescaler, DFF
- `SPI_SendData()` / `SPI_ReceiveData()` — polling-based TX/RX
- `SPI_SSOEConfig()` — hardware NSS output enable
- `SPI_CommunicationControl()` — SPE enable/disable

### USART (`usart.c`)
- `USART_Init()` — configures mode, word length, stop bits, parity, baud rate
- `USART_SetBaudRate()` — computes BRR from PCLK1/PCLK2
- `USART_SendData()` — polling TX with TXE and TC flag wait
- `USART_EnableComm()` — sets UE bit

### EXTI (`exti.c`)
- `Interrupt_Init()` — configures SYSCFG_EXTICR, RTSR/FTSR, IMR
- `NVIC_IRQConfig()` — enables/disables IRQ via ISER/ICER
- `NVIC_IRQPriorityConfig()` — sets priority in IPR register
- `NVIC_IRQHandling()` — clears EXTI_PR pending bit (W1C)

### RCC (`rcc.c`)
- `RCC_AHB1_Init()` / `RCC_APB1_Init()` / `RCC_APB2_Init()` — peripheral clock enable/disable
- `RCC_GetAPB1Clk()` / `RCC_GetAPB2Clk()` — returns peripheral bus clock frequency

### LIS3DSH (`lis3dsh.c`)
- `lis3dsh_Init()` — soft reset, sensor config, USART init
- `lis3dsh_INT1_Init()` — PE0 GPIO input, EXTI0 rising edge, NVIC IRQ6
- `lis3dsh_Read_XYZ()` — reads OUT_X/Y/Z registers, stores in `buffer[6]`
- `lis3dsh_LED_Init()` — initializes PD12–PD15 as outputs
- `lis3dsh_Green/Blue/Orange/Red_ON()` — exclusive LED drive for tilt indication

---

## LIS3DSH Configuration

| Register | Value | Description |
|---|---|---|
| CTRL_REG4 | 0x6F | ODR=100Hz, BDU=1, XYZ axes enabled |
| CTRL_REG5 | 0xC0 | Anti-aliasing filter BW = 50Hz |
| CTRL_REG3 | 0xC8 | DR_EN=1, IEA=1 (active high), INT1_EN=1 |

### SPI Configuration

| Parameter | Value |
|---|---|
| Mode | Master, Full Duplex |
| CPOL | 1 |
| CPHA | 1 |
| Data frame | 8 bit |
| Prescaler | /8 → 2MHz SPI clock |
| NSS | Hardware (SSOE=1) |
| CS | Software (PE3 GPIO output) |

---

## Interrupt Flow

```
LIS3DSH INT1 asserts HIGH (data ready at 100Hz)
        ↓
PE0 rising edge detected by EXTI0
        ↓
NVIC IRQ6 fires → EXTI0_IRQHandler()
        ↓
lis3dsh_Read_XYZ()     — reads 6 bytes via SPI, deasserts INT1
dataReady = 1          — sets flag for main loop
NVIC_IRQHandling(0)    — clears EXTI_PR bit 0 (W1C)
        ↓
main() while(1) detects dataReady
        ↓
Compute X/Y/Z in microg, apply tilt threshold, drive LED, send via USART2
```

---

## Tilt Detection

Sensitivity: `60 microg/LSB` (±2g full scale)

```c
#define X_OFFSET  1500    // zero-g offset calibration
#define Y_OFFSET  238000

int32_t x_comp = x_mg - X_OFFSET;
int32_t y_comp = y_mg - Y_OFFSET;

if(x_comp > 300000)        Red LED    // tilt right
else if(x_comp < -300000)  Green LED  // tilt left
else if(y_comp > 300000)   Blue LED   // tilt forward
else if(y_comp < -300000)  Orange LED // tilt backward
```

Threshold of 300000 microg (0.3g) corresponds to approximately 17° tilt.

---

## Serial Output

- **Interface:** USART2, PA2 TX
- **Baud rate:** 115200
- **Format:** `X: <val>microg ------- Y: <val>microg ------- Z: <val>microg`
- **Terminal settings:** 115200 baud, 8N1, **flow control = None**

---

## Startup Sequence (main)

```c
lis3dsh_Init();       // sensor + SPI + USART init, soft reset
lis3dsh_LED_Init();   // PD12–15 GPIO output init
lis3dsh_Read_XYZ();   // flush stale DR — deassert INT1 before arming EXTI
lis3dsh_INT1_Init();  // PE0 input, EXTI0 rising edge, NVIC IRQ6 enable
```

**Critical:** `lis3dsh_Read_XYZ()` must be called before `lis3dsh_INT1_Init()`. If INT1 is already HIGH when EXTI is armed, no rising edge is detected and the handler never fires.

---

## NVIC IRQ Map

| Interrupt | IRQn | Priority |
|---|---|---|
| EXTI0 (PE0 / LIS3DSH INT1) | 6 | 13 |

---

## Known Gotchas

- **BDU=1 requires reading L and H bytes of each axis** before the output latch releases and INT1 deasserts. Reads are done individually via `lis3dsh_Read_Reg()` — each call asserts/deasserts CS separately, which works because L and H bytes are read sequentially within the same axis before moving to the next.
- **LIS3DSH auto-increment (bit6 of address)** does not work for output registers 0x28–0x2D — burst read returns garbage. Use individual register reads.
- **Schematic label on STM32F4-Discovery says LIS302DL** — actual IC is LIS3DSH (confirmed via WHO_AM_I = 0x3F).
- **SYSCFG struct was missing MEMRMP** at offset 0x00 — caused all EXTICR writes to land on wrong registers.

---

## Build Environment

- **IDE:** STM32CubeIDE 1.18.1
- **Toolchain:** GNU Arm Embedded Toolchain 13.3
- **Language:** C (baremetal, no HAL, no CubeMX code generation)
- **Debug:** ST-Link V2 (onboard)
