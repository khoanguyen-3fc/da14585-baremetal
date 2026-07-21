// Define used registers
static volatile unsigned short &SET_FREEZE_REG =
    *reinterpret_cast<unsigned short *>(0x50003300);
static volatile unsigned short &CLK_PER_REG =
    *reinterpret_cast<unsigned short *>(0x50000004);

// PMU_CTRL_REG (0x50000010)
static volatile unsigned short &PMU_CTRL_REG =
    *reinterpret_cast<unsigned short *>(0x50000010);

// SYS_CTRL_REG (0x50000012)
static volatile unsigned short &SYS_CTRL_REG =
    *reinterpret_cast<unsigned short *>(0x50000012);

// SYS_STAT_REG (0x50000014)
static volatile unsigned short &SYS_STAT_REG =
    *reinterpret_cast<unsigned short *>(0x50000014);

// P04_MODE_REG (0x5000300E) — P0.4 pin mode / function select
static volatile unsigned short &P04_MODE_REG =
    *reinterpret_cast<unsigned short *>(0x5000300E);

// UART2 debug TX on P0.4, 115200 8-N-1 (mirrors provision/epd585)
static volatile unsigned short &UART2_THR_DLL =
    *reinterpret_cast<unsigned short *>(0x50001100); // TX holding / divisor low
static volatile unsigned short &UART2_IER_DLH =
    *reinterpret_cast<unsigned short *>(0x50001104); // divisor high
static volatile unsigned short &UART2_FCR =
    *reinterpret_cast<unsigned short *>(0x50001108); // FIFO control
static volatile unsigned short &UART2_LCR =
    *reinterpret_cast<unsigned short *>(0x5000110C); // line control
static volatile unsigned short &UART2_LSR =
    *reinterpret_cast<unsigned short *>(0x50001114); // line status
static volatile unsigned short &UART2_DLF =
    *reinterpret_cast<unsigned short *>(0x500011C0); // fractional divisor

void delay_us(int us) {
  while (us--) {
    __asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
  }
}

// UART2 TX bring-up + blocking puts (P0.4 = TX). Runs on the reset-default
// ~16 MHz RC clock; epd585 switches to XTAL16M for a precise baud, omitted
// here to keep the change minimal.
void uart_init() {
  CLK_PER_REG |= (1 << 6);  // UART2 peripheral clock enable
  P04_MODE_REG = 0x300 | 4; // P0.4 -> UART2 TX (output, function 4)

  UART2_LCR = 0x80;     // DLAB=1: expose divisor latches
  UART2_THR_DLL = 0x08; // divisor = 8  -> 115200 @ 16 MHz
  UART2_IER_DLH = 0x00; // divisor high = 0
  UART2_DLF = 0x0B;     // + 11/16 fractional divisor
  UART2_LCR = 0x03;     // DLAB=0, 8-N-1
  UART2_FCR = 0x01;     // enable FIFO
}

void uart_putc(char c) {
  for (volatile int i = 0; i < 100000; i++)
    if (UART2_LSR & 0x20)
      break; // wait for TX holding register empty
  UART2_THR_DLL = (unsigned char)c;
}

void uart_puts(const char *s) {
  while (*s)
    uart_putc(*s++);
}

// Application
int main() {
  SET_FREEZE_REG = (1 << 3); // FRZ_WDOG

  PMU_CTRL_REG = 1 << 2; // RADIO_SLEEP (clears PERIPH_SLEEP -> power PD_PER)
  while (!(SYS_STAT_REG & 0x8)) {
  } // wait until the peripheral domain is up
  SYS_CTRL_REG =
      0b101 << 5 | 0x2; // DEBUGGER_ENABLE, PAD_LATCH_EN, REMAP_ADR0=SysRAM1

  // Say hello over the P0.4 debug UART
  uart_init();
  uart_puts("\r\n[baremetal] hello from DA14585\r\n");

  for (int i = 0; i < 10; i++) {
    delay_us(200000); // 200 ms delay
    uart_puts("[baremetal] tick ");
    uart_putc('0' + i);
    uart_puts("\r\n");
  }

  // Enter deep sleep mode
  delay_us(1000000); // 1 second delay

  PMU_CTRL_REG = 0b11 << 1;    // RADIO_SLEEP, PERIPH_SLEEP
  SYS_CTRL_REG = 1 << 7 | 0x2; // DEBUGGER_ENABLE, REMAP_ADR0=SysRAM1

  // SCB->SCR |= (SCB_SCR_SLEEPDEEP_Msk);
  *((volatile unsigned int *)(0xE000ED10)) |= (1UL << 2);

  __asm volatile("nop");
  __asm volatile("nop");
  __asm volatile("nop");

  __asm volatile("wfi");
}

extern "C" {
void _start() {
  main();
  while (1) {
  }
}

void SystemInit() {}
}
