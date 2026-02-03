# Conectar a OpenOCD
target extended-remote localhost:3333

# Cargar símbolos del ELF
file build/freertos-stm32f4.elf

# Resetear y detener el chip
monitor reset halt

# Cargar el programa (opcional si ya flasheaste)
# load

# Poner breakpoint en main
break main

# Continuar hasta el breakpoint
continue
