# Export Hardware (XSA) without bitstream
# Target: D:/Zybo/project_Zybo_MemAcc/Zybo_MemAcc_wrapper.xsa

puts "INFO: Opening project..."
open_project D:/Zybo/project_Zybo_MemAcc/project_Zybo_MemAcc.xpr

puts "INFO: Opening BD design..."
open_bd_design [get_files design_1.bd]

puts "INFO: Generating wrapper (if needed)..."
set wrapper_file [get_files -quiet *wrapper.v]
if { $wrapper_file eq "" } {
    make_wrapper -files [get_files design_1.bd] -top
    add_files -norecurse [glob D:/Zybo/project_Zybo_MemAcc/project_Zybo_MemAcc.gen/sources_1/bd/design_1/hdl/design_1_wrapper.v]
    set_property top design_1_wrapper [current_fileset]
}

puts "INFO: Exporting hardware (XSA) without bitstream..."
write_hw_platform -fixed -force D:/Zybo/project_Zybo_MemAcc/Zybo_MemAcc_wrapper.xsa

puts "INFO: XSA export complete -> D:/Zybo/project_Zybo_MemAcc/Zybo_MemAcc_wrapper.xsa"
