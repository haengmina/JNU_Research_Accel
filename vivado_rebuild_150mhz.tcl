############################################################
## Vivado Rebuild Script: 150MHz Clock Timing Fix
## Project: project_Zybo_MemAcc
## Target: Zybo Z7-10 (xc7z010clg400-1)
## Generated: 2026-02-24
## Purpose: Resolve WNS -2.146ns by reducing clock to 150MHz
############################################################

# Open existing project
open_project D:/Zybo/project_Zybo_MemAcc/project_Zybo_MemAcc.xpr

# Upgrade IP to pick up updated XCI parameters (PS7 + memcopy_accel)
upgrade_ip [get_ips]

# Regenerate BD output products
set bd_file [get_files *.bd]
generate_target all $bd_file

# Reset previous runs to force full rebuild
reset_run synth_1
reset_run impl_1

# Run synthesis
launch_runs synth_1 -jobs 4
wait_on_run synth_1

# Check synthesis status
set synth_status [get_property STATUS [get_runs synth_1]]
puts "Synthesis status: $synth_status"

# Run implementation
launch_runs impl_1 -jobs 4
wait_on_run impl_1

# Check implementation status
set impl_status [get_property STATUS [get_runs impl_1]]
puts "Implementation status: $impl_status"

# Check timing closure (WNS)
open_run impl_1
set wns [get_property SLACK [get_timing_paths -max_paths 1 -nworst 1 -setup]]
puts "WNS (Worst Negative Slack): $wns ns"

if {$wns >= 0} {
    puts "TIMING CLOSURE ACHIEVED: WNS = $wns ns (>= 0)"
} else {
    puts "WARNING: Timing not closed. WNS = $wns ns"
}

# Generate timing report
report_timing_summary -file D:/Zybo/timing_report_150mhz.rpt

# Generate bitstream
launch_runs impl_1 -to_step write_bitstream -jobs 4
wait_on_run impl_1

puts "Bitstream generation complete."
puts "Bitstream location: D:/Zybo/project_Zybo_MemAcc/project_Zybo_MemAcc.runs/impl_1/"

# Export XSA
write_hw_platform -fixed -include_bit -force -file D:/Zybo/project_Zybo_MemAcc/Zybo_MemAcc_wrapper.xsa

puts "XSA exported to: D:/Zybo/project_Zybo_MemAcc/Zybo_MemAcc_wrapper.xsa"
puts "All done!"
