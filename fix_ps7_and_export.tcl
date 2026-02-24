open_project D:/Zybo/project_Zybo_MemAcc/project_Zybo_MemAcc.xpr
open_bd_design D:/Zybo/project_Zybo_MemAcc/project_Zybo_MemAcc.srcs/sources_1/bd/design_1/design_1.bd
set ps7 [get_bd_cells /processing_system7_0]

# Ensure UART1 is enabled
set_property -dict [list   CONFIG.PCW_UART1_PERIPHERAL_ENABLE {1}   CONFIG.PCW_UART1_UART1_IO {MIO 48 .. 49}   CONFIG.PCW_EN_UART1 {1} ] $ps7

validate_bd_design
save_bd_design

# Generate output products
generate_target all [get_files D:/Zybo/project_Zybo_MemAcc/project_Zybo_MemAcc.srcs/sources_1/bd/design_1/design_1.bd]

# Export Hardware
write_hw_platform -fixed -force -file D:/Zybo/project_Zybo_MemAcc/Zybo_MemAcc_wrapper.xsa

close_project
