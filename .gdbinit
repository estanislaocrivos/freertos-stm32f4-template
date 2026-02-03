# Connect to OpenOCD
target extended-remote localhost:3333

# Load symbols from ELF
file build/freertos-stm32f4.elf

# Reset and halt the chip
monitor reset halt

# Load the program (optional if already flashed)
# load

# Set breakpoint at main
break main

# Continue to breakpoint
continue
