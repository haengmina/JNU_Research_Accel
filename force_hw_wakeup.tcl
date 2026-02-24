connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~ "*A9*#0"}
rst -system
after 1000

# SLCR Unlock (Vital for register writing)
mwr -force 0xF8000008 0xDF0D

# Enable UART1 Peripheral Clock (Bit 21, 20 for UART1)
mwr -force 0xF800012C 0x016C000D
mwr -force 0xF8000154 0x00000A02

# Enable UART1 MIO (MIO 48, 49)
mwr -force 0xF8000B40 0x00000600
mwr -force 0xF8000B44 0x00000600

# Download and run (main bp to see if it stops)
dow D:/Zybo/MemAcc2_app/build/MemAcc2_app.elf
bpadd main
con
