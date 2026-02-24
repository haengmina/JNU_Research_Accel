connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~ "*A9*#0"}
puts "--- REGISTER DUMP START ---"
rrd pc
rrd lr
rrd sp
rrd cpsr
puts "--- VECTOR TABLE DUMP ---"
mrd 0x0 8
puts "--- UART1 CLOCK STATUS (0xF800012C) ---"
mrd 0xF800012C
puts "--- REGISTER DUMP END ---"
