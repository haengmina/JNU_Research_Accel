# =============================================================================
# enable_uart1_rebuild.tcl
# Purpose: Enable UART1 (MIO 48-49) on Zybo Z7-10, rebuild bitstream, export XSA
# Usage:   vivado -mode batch -source enable_uart1_rebuild.tcl
# =============================================================================
set proj_dir "D:/Zybo/project_Zybo_MemAcc"
set proj_name "project_Zybo_MemAcc"
set bd_name   "design_1"
set xsa_output "D:/Zybo/project_Zybo_MemAcc/Zybo_MemAcc_wrapper.xsa"
puts "============================================================"
puts " HW-Lab: UART1 Enable Script Starting"
puts " Project: $proj_dir/$proj_name.xpr"
puts "============================================================"

# Step 1: Open project
puts "\nStep 1: Opening project..."
open_project "$proj_dir/$proj_name.xpr"
# Step 2: Open Block Design
puts "\nStep 2: Opening Block Design..."
open_bd_design "$proj_dir/${proj_name}.srcs/sources_1/bd/${bd_name}/${bd_name}.bd"
# Step 3: Enable UART1 on PS7 (MIO 48=TX, MIO 49=RX)
puts "\nStep 3: Configuring PS7 UART1..."
set ps7_cell [get_bd_cells processing_system7_0]
set_property -dict [list \
    CONFIG.PCW_UART1_PERIPHERAL_ENABLE  {1} \
    CONFIG.PCW_UART1_UART1_IO           {MIO 48 .. 49} \
    CONFIG.PCW_EN_UART1                 {1} \
    CONFIG.PCW_UART_PERIPHERAL_VALID    {1} \
    CONFIG.PCW_UART_PERIPHERAL_FREQMHZ  {100} \
    CONFIG.PCW_UART1_BAUD_RATE          {115200} \
    CONFIG.PCW_ACT_UART_PERIPHERAL_FREQMHZ {100.000000} \
] $ps7_cell
puts "  -> UART1 enabled: MIO 48=TX, MIO 49=RX, 115200 baud"

# Step 4: Validate Block Design
puts "\nStep 4: Validating Block Design..."
validate_bd_design
# Step 5: Save Block Design
puts "\nStep 5: Saving Block Design..."
save_bd_design
# Step 6: Regenerate HDL Wrapper
puts "\nStep 6: Regenerating HDL Wrapper..."
set wrapper_file [make_wrapper -files [get_files "${bd_name}.bd"] -top]
set old_wrapper [get_files -quiet "*${bd_name}_wrapper.v"]
if {$old_wrapper ne ""} {
    remove_files $old_wrapper
}
add_files -norecurse $wrapper_file
set_property top ${bd_name}_wrapper [current_fileset]
# Step 7: Upgrade IPs if needed
puts "\nStep 7: Checking IP upgrades..."
upgrade_ip [get_ips -quiet] -quiet
# Step 8: Synthesis
puts "\nStep 8: Running Synthesis (this may take several minutes)..."
reset_run synth_1
launch_runs synth_1 -jobs 4
wait_on_run synth_1
if {[get_property PROGRESS [get_runs synth_1]] != "100%"} {
    error "ERROR: Synthesis failed. Check logs."
}
puts "  -> Synthesis complete"

# Step 9: Implementation
puts "\nStep 9: Running Implementation..."
reset_run impl_1
launch_runs impl_1 -jobs 4
wait_on_run impl_1
if {[get_property PROGRESS [get_runs impl_1]] != "100%"} {
    error "ERROR: Implementation failed. Check logs."
}
puts "  -> Implementation complete"

# Step 10: Generate Bitstream
puts "\nStep 10: Generating Bitstream..."
launch_runs impl_1 -to_step write_bitstream -jobs 4
wait_on_run impl_1
if {[get_property PROGRESS [get_runs impl_1]] != "100%"} {
    error "ERROR: Bitstream generation failed. Check logs."
}
puts "  -> Bitstream generation complete"

# Step 11: Export XSA
puts "\nStep 11: Exporting XSA..."
write_hw_platform -fixed -include_bit -force -file $xsa_output
puts "  -> XSA exported: $xsa_output"

# Done
puts "\n============================================================"
puts " ALL STEPS COMPLETE"
puts " XSA: $xsa_output"
puts " UART1: MIO 48 (TX) / MIO 49 (RX) @ 115200 baud"
puts "============================================================"
close_project
