# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "D:\\Lab\\Project\\FPGA_accel\\workspace\\hm_workspace\\zcu104_platform\\psu_cortexa53_0\\standalone_psu_cortexa53_0\\bsp\\include\\diskio.h"
  "D:\\Lab\\Project\\FPGA_accel\\workspace\\hm_workspace\\zcu104_platform\\psu_cortexa53_0\\standalone_psu_cortexa53_0\\bsp\\include\\ff.h"
  "D:\\Lab\\Project\\FPGA_accel\\workspace\\hm_workspace\\zcu104_platform\\psu_cortexa53_0\\standalone_psu_cortexa53_0\\bsp\\include\\ffconf.h"
  "D:\\Lab\\Project\\FPGA_accel\\workspace\\hm_workspace\\zcu104_platform\\psu_cortexa53_0\\standalone_psu_cortexa53_0\\bsp\\include\\sleep.h"
  "D:\\Lab\\Project\\FPGA_accel\\workspace\\hm_workspace\\zcu104_platform\\psu_cortexa53_0\\standalone_psu_cortexa53_0\\bsp\\include\\xilffs.h"
  "D:\\Lab\\Project\\FPGA_accel\\workspace\\hm_workspace\\zcu104_platform\\psu_cortexa53_0\\standalone_psu_cortexa53_0\\bsp\\include\\xilffs_config.h"
  "D:\\Lab\\Project\\FPGA_accel\\workspace\\hm_workspace\\zcu104_platform\\psu_cortexa53_0\\standalone_psu_cortexa53_0\\bsp\\include\\xiltimer.h"
  "D:\\Lab\\Project\\FPGA_accel\\workspace\\hm_workspace\\zcu104_platform\\psu_cortexa53_0\\standalone_psu_cortexa53_0\\bsp\\include\\xtimer_config.h"
  "D:\\Lab\\Project\\FPGA_accel\\workspace\\hm_workspace\\zcu104_platform\\psu_cortexa53_0\\standalone_psu_cortexa53_0\\bsp\\lib\\libxilffs.a"
  "D:\\Lab\\Project\\FPGA_accel\\workspace\\hm_workspace\\zcu104_platform\\psu_cortexa53_0\\standalone_psu_cortexa53_0\\bsp\\lib\\libxiltimer.a"
  )
endif()
