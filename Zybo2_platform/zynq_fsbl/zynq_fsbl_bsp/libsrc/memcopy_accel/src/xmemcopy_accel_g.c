#include "xmemcopy_accel.h"

XMemcopy_accel_Config XMemcopy_accel_ConfigTable[] __attribute__ ((section (".drvcfg_sec"))) = {

	{
		"xlnx,memcopy-accel-1.0", /* compatible */
		0x40000000 /* reg */
	},
	 {
		 NULL
	}
};