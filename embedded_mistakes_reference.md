# Embedded Systems — Mistakes & Reference Guide
### STM32F4 Baremetal Development (LIS3DSH/SPI/EXTI/NVIC)

---

## 1. C Language — Linkage & Storage Class

### 1.1 `static` at file scope = internal linkage
`static` makes a symbol **invisible outside the translation unit**. The linker cannot resolve `extern` references to it from other `.c` files.

```c
// lis3dsh.c
static USART_Handle_t usart2;  // ❌ invisible to linker
static uint8_t buffer[6];      // ❌ invisible to linker
```

**Rule:** Variables shared across translation units must NOT be `static`.

---

### 1.2 Definitions in header files = multiple definition linker error
A header included in N translation units means N definitions of the same symbol — the linker rejects this.

```c
// lis3dsh.h
USART_Handle_t usart2;   // ❌ definition in header — included in multiple .c files
uint8_t buffer[6];       // ❌ same problem
```

**Fix — declare in header, define in one .c file:**

```c
// lis3dsh.h
extern USART_Handle_t usart2;   // ✅ declaration only
extern uint8_t buffer[6];       // ✅ declaration only

// lis3dsh.c
USART_Handle_t usart2;          // ✅ single definition
uint8_t buffer[6];              // ✅ single definition
```

---

### 1.3 `volatile` for ISR-shared variables
Variables written in an ISR and read in `main()` must be `volatile`. Without it, the compiler may cache the value in a register or optimize out the check entirely.

```c
uint8_t dataReady = 0;          // ❌ compiler may optimize away the while(1) check
volatile uint8_t dataReady = 0; // ✅
```

---

### 1.4 Array pointer vs address-of-array
```c
char msg[200];
USART_SendData(&usart2, (uint8_t*)&msg, strlen(msg));  // ❌ &msg = address of array object
USART_SendData(&usart2, (uint8_t*)msg,  strlen(msg));  // ✅ msg decays to pointer to first element
```

---

### 1.5 `uint8_t` overflow in shift operations
Shifting a `uint8_t` value by more than 7 bits causes overflow — the result is truncated to 8 bits before the shift completes.

```c
uint8_t val = 0x4;
uint8_t pos = 12;
uint8_t result = val << pos;  // ❌ overflow — result is 0
uint32_t result = (uint32_t)val << pos;  // ✅ cast before shift
```

**Rule:** Always cast to `uint32_t` before shifting into register fields.

---

## 2. Register-Level Programming

### 2.1 Read-Modify-Write — always clear before set
Never assume a bitfield is zero before writing. OR-only cannot turn bits off.

```c
// Wrong — old bits OR'd with new
REG |= (VAL << POS);

// Correct — clear field first, then set
REG &= ~(MASK << POS);
REG |=  (VAL  << POS);
```

**Example — OR cannot correct a wrong priority:**
```
Old:  1010_0000
New:  0100_0000
OR:   1110_0000  ← wrong
```

---

### 2.2 Write-1-to-Clear (W1C) registers
Some registers (e.g. EXTI_PR) clear a bit by **writing 1 to it**, not 0. Writing to a local copy has no effect — must write directly to the register.

```c
// ❌ local copy — register never cleared
volatile uint32_t pendReg = EXTI->EXTI_PR;
pendReg |= (1U << pinNum);

// ✅ write directly to register
EXTI->EXTI_PR |= (1U << pinNum);
```

---

### 2.3 Assignment vs OR in multi-field registers
Using `=` instead of `|=` on a register shared by multiple fields wipes all other fields.

```c
// EXTICR1 holds 4 pin mappings × 4 bits each
SYSCFG->SYSCFG_EXTICR[reg] = (val << pos);   // ❌ clears other 3 pin mappings
SYSCFG->SYSCFG_EXTICR[reg] |= (val << pos);  // ✅ preserves other fields
```

---

### 2.4 Peripheral clock must be enabled before register access
Any read or write to a peripheral's registers before its clock is enabled is silently ignored.

```c
// SYSCFG on APB2, bit 14
RCC->APB2ENR |= (1 << 14);  // must come before any SYSCFG register access
```

Same applies to GPIOA, GPIOE, SPI1, USART2 — always enable RCC clock first.

---

## 3. Struct Register Definitions

### 3.1 Missing registers shift all subsequent fields
If a peripheral register is missing from the struct, every subsequent field maps to the wrong hardware address.

**Incorrect SYSCFG struct — missing MEMRMP:**
```c
typedef struct {
    volatile uint32_t SYSCFG_PMC;       // maps to 0x00 — wrong, should be MEMRMP
    volatile uint32_t SYSCFG_EXTICR[4]; // maps to 0x04 — wrong, should start at 0x08
} Syscfg_RegDef_t;
```

**Correct:**
```c
typedef struct {
    volatile uint32_t SYSCFG_MEMRMP;    // 0x00
    volatile uint32_t SYSCFG_PMC;       // 0x04
    volatile uint32_t SYSCFG_EXTICR[4]; // 0x08–0x14
    uint32_t          RESERVED1[2];     // 0x18–0x1C
    volatile uint32_t SYSCFG_CMPCR;     // 0x20
} Syscfg_RegDef_t;
```

**Rule:** Always verify struct layout against the reference manual (RM0090) register map.

---

## 4. NVIC Configuration

### 4.1 ISER/ICER boundary — off-by-one
Each ISERx/ICERx register holds 32 IRQ bits (0–31, 32–63, 64–95).

```c
if(IRQNumber < 31)   // ❌ misses IRQNumber == 31
if(IRQNumber < 32)   // ✅
```

### 4.2 Bit offset within ISERx — always %32
```c
*NVIC_ISER2 |= (1U << IRQNumber % 64);  // ❌ wrong for 32-bit register
*NVIC_ISER2 |= (1U << IRQNumber % 32);  // ✅
```

### 4.3 IRQ numbers are not pin numbers
EXTI0 IRQ number on STM32F4 is **6**, not 0.

```
EXTI0  → IRQn = 6
EXTI1  → IRQn = 7
EXTI2  → IRQn = 8
EXTI3  → IRQn = 9
EXTI4  → IRQn = 10
```

Pass `6` to `NVIC_IRQConfig`, not `0`.

### 4.4 IPR register bit position
Each IPR register holds 4 IRQ priorities × 8 bits. STM32F4 implements only the upper 4 bits of each byte.

```c
uint8_t reg    = IRQNumber / 4;
uint8_t pos    = (IRQNumber % 4) * 8;
uint8_t bitPos = pos + (8 - STM32_IMPLEMENTED_BITS);  // shift to upper bits

*(NVIC_IPR0 + reg) &= ~(0xFF << bitPos);  // clear first
*(NVIC_IPR0 + reg) |=  (IRQPriority << bitPos);
```

---

## 5. EXTI Configuration

### 5.1 Full EXTI init sequence (order matters)
```
1. Enable GPIO clock
2. Configure GPIO pin as input
3. Enable SYSCFG clock
4. Configure SYSCFG_EXTICR — map pin to port
5. Configure EXTI RTSR/FTSR — trigger edge
6. Configure EXTI IMR — unmask line
7. Configure NVIC ISER — enable IRQ
8. Configure NVIC IPR — set priority
```

### 5.2 Trigger selection must match signal polarity
| Sensor/signal config | Trigger |
|---|---|
| Active high (IEA=1) | `EXTI_RISING_TRIGGER_ONLY` |
| Active low (IEA=0) | `EXTI_FALLING_TRIGGER_ONLY` |
| Button to GND + pull-up | `EXTI_FALLING_TRIGGER_ONLY` |
| Button to VCC + pull-down | `EXTI_RISING_TRIGGER_ONLY` |

### 5.3 EXTI only detects transitions, not static levels
If a pin is already HIGH when EXTI rising edge trigger is armed, no interrupt fires — the transition already happened before EXTI was configured.

**Fix:** Ensure the pin is LOW before arming EXTI, or use SWIER to force a software trigger for testing.

```c
EXTI->EXTI_SWIER |= (1U << pinNum);  // software trigger — bypasses physical signal
```

### 5.5 Initialization order — sensor DR must be cleared before arming EXTI
If the sensor's data ready flag is already set when `Interrupt_Init()` runs, INT1 is already HIGH. EXTI never sees a rising edge — handler never fires.

**Wrong order:**
```c
lis3dsh_Init();      // sensor starts, INT1 may already assert HIGH
lis3dsh_INT1_Init(); // EXTI armed — but PE0 already HIGH, no rising edge ever detected
```

**Correct order:**
```c
lis3dsh_Init();      // sensor configured
lis3dsh_Read_XYZ();  // clears DR → INT1 deasserts → PE0 goes LOW
lis3dsh_INT1_Init(); // EXTI armed — PE0 is LOW, next DR fires clean rising edge
```

**Rule:** Always ensure the interrupt source pin is in its idle/deasserted state before configuring EXTI. For any sensor with a data ready interrupt, perform at least one dummy read after sensor init and before EXTI init to guarantee the line is low when the edge trigger is armed.

This issue recurs any time the system restarts or re-initializes without clearing the sensor's pending DR flag first.

### 5.4 EXTI_PR pending bit must be cleared in handler
If PR is not cleared, the NVIC sees the interrupt as still pending.

```c
void NVIC_IRQHandling(uint8_t pinNum)
{
    if(EXTI->EXTI_PR & (1U << pinNum))
        EXTI->EXTI_PR |= (1U << pinNum);  // W1C — write 1 to clear
}
```

---

## 6. GPIO Configuration

### 6.1 Output vs input mode
- `GPIO_input` → MCU **samples** the pin; ODR writes are ignored
- `GPIO_output` → MCU **drives** the pin high or low via ODR

A CS (chip select) pin must be **output**. Configuring it as input means CS never asserts — the peripheral never responds, all reads return `0xFF`.

### 6.2 Pull-up/pull-down selection
| Situation | Config |
|---|---|
| Externally driven signal (sensor output, driven CS) | `GPIO_noPupd` |
| Floating input (button, no external resistor) | `GPIO_PuUp` or `GPIO_PuDn` |
| Button to GND | `GPIO_PuUp` |
| Button to VCC | `GPIO_PuDn` |

### 6.3 Alternate function requires correct AF number
SPI1 on PA5/PA6/PA7 uses **AF5**. USART2 on PA2 uses **AF7**. Wrong AF number = peripheral does not receive/drive the pin.

---

## 7. SPI Configuration

### 7.1 Hardware NSS vs software CS
With `SPI_ssm = SPI_ssmDi` (hardware NSS, SSM=0), SSOE=1 causes NSS to auto-assert on transmission. If using a separate GPIO as CS (software control), configure that pin as **GPIO output** and toggle manually.

### 7.2 BDU=1 requires burst read for correct behavior
With BDU (Block Data Update) = 1 on LIS3DSH, the output register latch is only released after **both L and H bytes of each axis are read in the same SPI transaction** (CS held low). Individual reads with CS toggling between each byte do not release the latch — DR stays set, INT1 stays asserted permanently.

**Fix — burst read with auto-increment (bit6 of address):**
```c
uint8_t addr = LIS3DSH_OUT_X_L | 0x80 | 0x40;  // read + auto-increment
lis3dsh_CSS_Enable();
SPI_SendData(SPI1, &addr, 1);
SPI_ReceiveData(SPI1, &flush, 1);
for(uint8_t i = 0; i < 6; i++) {
    uint8_t dummy = 0x00;
    SPI_SendData(SPI1, &dummy, 1);
    SPI_ReceiveData(SPI1, &buffer[i], 1);
}
lis3dsh_CSS_Disable();
```

### 7.3 SPI read/write protocol for LIS3DSH
```
bit7 of address byte:
  1 = read
  0 = write

bit6 of address byte:
  1 = auto-increment (burst mode)
  0 = single register
```

---

## 8. Sensor Configuration (LIS3DSH)

### 8.1 Verify WHO_AM_I before trusting register reads
- LIS3DSH: `0x3F`
- LIS302DL: `0x3B`

Schematic labels can be wrong. Always read WHO_AM_I to confirm the actual IC.

### 8.2 IEA bit controls INT1 polarity
`CTRL_REG3` bit6 = IEA (Interrupt Enable Active):
- `IEA=0` → INT1 active **low** → use falling edge trigger
- `IEA=1` → INT1 active **high** → use rising edge trigger

`0x88 = 0b1000_1000` → IEA=0 (active low) ← common mistake when using rising trigger
`0xC8 = 0b1100_1000` → IEA=1 (active high) ← correct for rising edge

### 8.3 DR_EN routes data ready to INT1
`CTRL_REG3` bit7 = DR_EN. When set, INT1 pulses on every new data ready event at the configured ODR rate. No movement required — interrupt fires continuously.

### 8.4 Data overrun means nobody is reading fast enough
`STATUS_REG = 0xF0` means ZYXOR=1 — new data arrived before previous was read. Increase read rate or reduce ODR.

---

## 9. ISR Best Practices

### 9.1 No blocking operations inside ISR
USART transmission at 115200 baud for 50 bytes takes ~4ms. At 100Hz ODR (10ms period), blocking USART inside the ISR consumes 40% of CPU time in interrupt context — causes missed interrupts and system instability.

**Pattern — flag + main loop:**
```c
volatile uint8_t dataReady = 0;

void EXTI0_IRQHandler(void)
{
    lis3dsh_Read_XYZ();      // fast SPI read
    dataReady = 1;           // set flag
    NVIC_IRQHandling(0);     // clear pending bit
}

int main(void)
{
    // init...
    while(1)
    {
        if(dataReady)
        {
            dataReady = 0;
            // USART send, processing — safe here
        }
    }
}
```

### 9.2 No peripheral re-initialization inside frequently called functions
Calling `GPIO_Init()`, `SPI_Init()`, or similar inside a function called from an ISR or at high rate wastes cycles and can corrupt peripheral state.

### 9.3 Clear pending bit after application logic, not before
Clearing PR before reading sensor data is acceptable, but ensure the sensor read actually deasserts INT1 — otherwise the next rising edge never occurs.

### 9.4 Serial terminal configuration — always use these settings for bare UART

| Setting | Correct value |
|---|---|
| Baud rate | match `USART_BaudRate` in code |
| Data bits | 8 |
| Stop bits | 1 |
| Parity | None |
| Flow control | **None** |

**XON/XOFF flow control** is software handshaking — the terminal waits for control characters (0x11/0x13) from the device before displaying data. Since STM32 bare UART never sends these, the terminal displays nothing despite USART transmitting correctly. Always set flow control to None.

---

### 9.5 ISR handler must be at file scope
`EXTI0_IRQHandler` must be a global function. The vector table entry is a weak symbol aliased to `Default_Handler` — your definition overrides it at link time only if it has external linkage (global scope).

```c
// ✅ file scope — overrides weak symbol in vector table
void EXTI0_IRQHandler(void) { ... }

int main(void) {
    // ❌ never define ISR handlers inside main or any other function
}
```

---

## 10. Debugging Checklist for STM32 Interrupts

When an interrupt is configured but the handler never fires, check each layer:

| Layer | Register | Address | Expected value |
|---|---|---|---|
| GPIO mode | GPIOx_MODER | — | Input (00) |
| SYSCFG mapping | SYSCFG_EXTICR[n] | 0x40013808+ | Port value (e.g. 0x4 for PE) |
| EXTI trigger | EXTI_RTSR or FTSR | 0x40013C08/0C | bit set for pin |
| EXTI unmask | EXTI_IMR | 0x40013C00 | bit set for pin |
| EXTI pending | EXTI_PR | 0x40013C14 | bit sets on edge |
| NVIC enable | NVIC_ISER0 | 0xE000E100 | bit set for IRQn |
| Physical signal | GPIOx_IDR | — | toggling |

**If PR never sets** → physical signal not arriving (pin stuck, sensor not asserting)
**If PR sets but handler doesn't fire** → NVIC not enabled or wrong IRQn
**If handler fires once then stops** → PR not cleared, or signal not re-arming

---

## 11. Common Mistakes Summary Table

| Mistake | Effect | Fix |
|---|---|---|
| Definition in header | Multiple definition linker error | `extern` in header, define in one `.c` |
| `static` on shared variable | Linker can't resolve `extern` | Remove `static` |
| Missing `volatile` on ISR-shared var | Compiler optimizes out check | Add `volatile` |
| `uint8_t` shift overflow | Wrong register value | Cast to `uint32_t` before shift |
| OR without clear on bitfield | Old bits corrupt new value | Clear with `&= ~mask` first |
| W1C on local copy | Register never cleared | Write directly to register |
| Assignment on shared register | Other fields wiped | Use `|=` not `=` |
| Missing register in struct | All subsequent fields misaligned | Verify against RM0090 |
| CS pin as input | SPI ignored, all reads 0xFF | Configure CS as GPIO output |
| EXTI armed while pin already HIGH | No rising edge detected, handler never fires | Read sensor DR registers before calling EXTI init |
| Wrong init order (EXTI before sensor DR clear) | INT1 stuck HIGH, handler never fires, recurs on every restart | Call Read_XYZ() after sensor init, before EXTI init |
| Individual reads with BDU=1 | DR latch never released, INT1 stuck high | Burst read all bytes in one CS assertion |
| Blocking USART in ISR | Missed interrupts, system instability | Use flag + process in main loop |
| Wrong trigger polarity | Edge never detected | Match trigger to IEA bit / signal polarity |
| IRQn confusion (pin vs IRQ number) | Wrong NVIC bit set | EXTI0=6, EXTI1=7, EXTI2=8, EXTI3=9, EXTI4=10 |
| `< 31` instead of `< 32` | IRQ31 never enabled/disabled | Use `< 32` for ISER0/ICER0 boundary |
| `%64` instead of `%32` for ISER2 | Wrong bit position in register | Each ISERx is 32-bit wide, always `% 32` |
| Re-init peripherals in hot path | Corruption, wasted cycles | Init once in main, never in ISR or sensor read |
| SWIER used as permanent fix | Handler fires once then never again | SWIER only tests EXTI/NVIC chain — fix the physical signal instead |
| PuTTY flow control set to XON/XOFF | No output visible despite USART transmitting correctly | Always set flow control to None for bare UART with no handshaking |
| No soft reset on sensor init | Non-deterministic register state on restart | Always issue soft reset before configuring sensor |
| FIFO enabled unintentionally | INT1 asserts from FIFO fill, not DR | Explicitly write 0x00 to FIFO control register |
| INT configured before ODR/axes | Sensor may assert INT during configuration | Write INT register last in sensor init sequence |

---

## 12. Sensor State Management — Non-Deterministic Power-On State

### 12.1 Why this problem exists

When the MCU resets (debug restart, power cycle, watchdog), **the sensor does not necessarily reset**. It retains its register state from the previous session — including pending interrupt flags, FIFO contents, stale output data, and control register values. The MCU boots fresh but the sensor is mid-operation, causing INT1 to be already HIGH, FIFO partially filled, or DR flag set before init code runs.

This produces behavior that changes between debug sessions — sometimes works, sometimes doesn't — because sensor state at MCU reset is non-deterministic.

---

### 12.2 Safe sensor init sequence — general template

```c
void sensor_Init()
{
    sensor_CommEnable();             // 1. clocks, GPIO, SPI/I2C peripheral init

    sensor_SoftReset();              // 2. write reset bit — clears all regs to default
    for(volatile uint32_t i = 0;     // 3. delay for reset to complete
        i < 10000; i++);

    sensor_WriteReg(CONFIG1, val);   // 4. write ALL config registers explicitly
    sensor_WriteReg(CONFIG2, val);
    sensor_WriteReg(FIFO_REG, 0x00); // 5. explicitly disable FIFO if it exists
    sensor_WriteReg(INT_REG,  val);  // 6. configure INT last — after everything else

    sensor_VerifyRegs();             // 7. readback critical registers to confirm writes
}

int main(void)
{
    sensor_Init();
    sensor_FlushPendingData();       // 8. clear DR, flush FIFO, read status — before EXTI
    exti_Init();                     // 9. arm EXTI only after signal is guaranteed low

    while(1)
    {
        if(dataReady)
        {
            dataReady = 0;
            // process data
        }
    }
}
```

---

### 12.3 Rule — always soft reset first

Most sensors have a reset bit. Write it before any configuration:

```c
// LIS3DSH — STRT bit in CTRL_REG3
lis3dsh_Write_Reg(LIS3DSH_CTRL_REG3, 0x01);
for(volatile uint32_t i = 0; i < 10000; i++);  // wait for reset
```

After reset, all registers return to datasheet default values — known state guaranteed regardless of prior session.

---

### 12.4 Rule — explicitly write every register you depend on

Never rely on reset defaults for registers your code depends on, even if the default is correct. Write them all explicitly so the state is deterministic and self-documenting.

```c
lis3dsh_Write_Reg(LIS3DSH_CTRL_REG4, 0x6F);  // ODR, BDU, axes
lis3dsh_Write_Reg(LIS3DSH_CTRL_REG5, 0xC0);  // anti-aliasing filter
lis3dsh_Write_Reg(LIS3DSH_CTRL_REG6, 0x00);  // FIFO disabled — explicit
lis3dsh_Write_Reg(LIS3DSH_CTRL_REG3, 0xC8);  // INT1 last
```

**Why INT last:** configuring INT before ODR/axes means the sensor may assert INT during configuration before you are ready to handle it.

---

### 12.5 Rule — read back critical registers after writing

If SPI is broken (wrong CS, wrong CPOL/CPHA, wrong AF), writes are silently ignored and reads return `0xFF`. Verify early:

```c
uint8_t r4 = lis3dsh_Read_Reg(LIS3DSH_CTRL_REG4);
// in debugger: r4 must equal 0x6F
// if r4 == 0xFF → SPI communication is broken
// if r4 == 0x00 → write silently failed
```

---

### 12.6 Rule — flush pending sensor data before arming EXTI

Even after soft reset, one ODR cycle may complete before EXTI is armed. Always flush before `Interrupt_Init()`:

```c
lis3dsh_Init();
lis3dsh_Read_XYZ();               // releases BDU latch, clears DR flag
lis3dsh_Read_Reg(LIS3DSH_STATUS); // clear status register
lis3dsh_INT1_Init();              // arm EXTI — signal now guaranteed low
```

---

### 12.7 SWIER is a diagnostic tool, not a fix

`EXTI_SWIER` fires the handler once in software, bypassing the physical signal entirely:

```c
EXTI->EXTI_SWIER |= (1U << 0);  // fires EXTI0 handler once — diagnostic only
```

**Use it to:** verify the EXTI/NVIC chain is correctly configured when the physical signal is not arriving.

**Never use it as:** a permanent solution. After the one software trigger, INT1 is still stuck HIGH and no further rising edges occur — the handler fires exactly once then never again.

If SWIER works but the physical signal does not trigger the handler, the problem is always in the sensor state or signal path — not in EXTI/NVIC config.
