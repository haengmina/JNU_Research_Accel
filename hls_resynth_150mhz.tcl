############################################################
## HLS Re-synthesis Script: 150MHz (6.66ns) Clock Constraint
## Target: memcopy_accel IP for Zybo Z7-10 (xc7z010-clg400-1)
## Generated: 2026-02-24
############################################################
open_project MemAcc
set_top memcopy_accel
add_files src/Vitis-HLS/memcopy_accel.cpp
add_files -tb src/Vitis-HLS/memcopy_accel_tb_data.h
add_files -tb src/Vitis-HLS/memcopy_accel_test.cpp
open_solution "solution1" -flow_target vivado
set_part {xc7z010-clg400-1}
create_clock -period 6.66 -name default
config_export -display_name {Memcopy Accel} -output D:/Zybo
source "./MemAcc/solution1/directives.tcl"
csynth_design
export_design -rtl verilog -format ip_catalog -output D:/Zybo
