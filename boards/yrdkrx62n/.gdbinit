target remote localhost:2331
monitor halt
monitor flash device = RX62N
monitor flash download = 1
load AliOS.elf
monitor reset
