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

// P0_DATA_REG (0x50003000)
// P0_SET_DATA_REG (0x50003002)
// P0_RESET_DATA_REG (0x50003004)
static volatile unsigned short &P0_DATA_REG =
    *reinterpret_cast<unsigned short *>(0x50003000);
static volatile unsigned short &P0_SET_DATA_REG =
    *reinterpret_cast<unsigned short *>(0x50003002);
static volatile unsigned short &P0_RESET_DATA_REG =
    *reinterpret_cast<unsigned short *>(0x50003004);
static volatile unsigned short &P04_MODE_REG =
    *reinterpret_cast<unsigned short *>(0x5000300E);

void delay_us(int us) {
  while (us--) {
    __asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
  }
}

// Application
int main() {
  SET_FREEZE_REG = (1 << 3); // FRZ_WDOG

  PMU_CTRL_REG = 1 << 2;     // RADIO_SLEEP
  SYS_CTRL_REG = 0b101 << 5; // DEBUGGER_ENABLE, PAD_LATCH_EN

  // Using pin GPIO0.4 as output
  P04_MODE_REG = 0b11 << 8; // Set P0.4 as output

  // Blink P0.4 five times
  for (int i = 0; i < 5; i++) {
    P0_SET_DATA_REG = (1 << 4);   // Set P0.4
    delay_us(200000);             // 200 ms delay
    P0_RESET_DATA_REG = (1 << 4); // Clear P0.4
    delay_us(200000);             // 200 ms delay
  }

  // Enter deep sleep mode
  delay_us(1000000); // 1 second delay

  PMU_CTRL_REG = 0b11 << 1; // RADIO_SLEEP, PERIPH_SLEEP
  SYS_CTRL_REG = 1 << 7;    // DEBUGGER_ENABLE

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
