CFLAGS=-mthumb -mcpu=cortex-m0 -nostdlib -nostartfiles -Os
LDFLAGS=-T da14585.ld -nostdlib -Wl,--gc-sections -Wl,-Map,baremetal.map

all: baremetal.elf baremetal.bin baremetal.hex

baremetal.elf: startup.o main.o
	arm-none-eabi-gcc $(LDFLAGS) -o $@ $^

%.o: %.cpp
	arm-none-eabi-gcc $(CFLAGS) -c -o $@ $<

%.o: %.c
	arm-none-eabi-gcc $(CFLAGS) -c -o $@ $<

%.o: %.s
	arm-none-eabi-gcc -x assembler-with-cpp $(CFLAGS) -c -o $@ $<

baremetal.bin: baremetal.elf
	arm-none-eabi-objcopy -O binary $< $@

baremetal.hex: baremetal.elf
	arm-none-eabi-objcopy -O ihex $< $@

clean:
	rm -f *.o *.map *.elf *.bin *.hex

run: baremetal.hex
	openocd -f interface/cmsis-dap.cfg -f da14585.cfg \
	  -c "init; halt; load_image baremetal.hex; reset; resume; exit"

.PHONY: all clean run
