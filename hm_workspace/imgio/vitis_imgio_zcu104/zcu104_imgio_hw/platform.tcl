# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/mini/jnu/lab/ml/hm_workspace/imgio/vitis_imgio_zcu104/zcu104_imgio_hw/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/mini/jnu/lab/ml/hm_workspace/imgio/vitis_imgio_zcu104/zcu104_imgio_hw/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {zcu104_imgio_hw}\
-hw {/home/mini/jnu/lab/ml/hm_workspace/imgio/project_zcu104_imgio/zcu104_imgio_hw.xsa}\
-arch {64-bit} -fsbl-target {psu_cortexa53_0} -out {/home/mini/jnu/lab/ml/hm_workspace/imgio/vitis_imgio_zcu104}

platform write
domain create -name {standalone_psu_cortexa53_0} -display-name {standalone_psu_cortexa53_0} -os {standalone} -proc {psu_cortexa53_0} -runtime {cpp} -arch {64-bit} -support-app {hello_world}
platform generate -domains 
platform active {zcu104_imgio_hw}
domain active {zynqmp_fsbl}
domain active {zynqmp_pmufw}
domain active {standalone_psu_cortexa53_0}
platform generate -quick
platform generate
platform generate
platform active {zcu104_imgio_hw}
bsp reload
bsp setlib -name xilffs -ver 4.8
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_psu_cortexa53_0 
