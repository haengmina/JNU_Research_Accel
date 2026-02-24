open_project D:/Zybo/project_Zybo_MemAcc/project_Zybo_MemAcc.xpr
open_bd_design [get_files *.bd]
set ps7 [get_bd_cells /processing_system7_0]
puts "--- UART CHECK START ---"
puts "UART1 ENABLE: [get_property CONFIG.PCW_UART1_PERIPHERAL_ENABLE $ps7]"
puts "UART1 MIO: [get_property CONFIG.PCW_UART1_UART1_IO $ps7]"
puts "UART0 ENABLE: [get_property CONFIG.PCW_UART0_PERIPHERAL_ENABLE $ps7]"
puts "--- UART CHECK END ---"
close_project
