/******************************************************************************
* Copyright (c) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XIL_XARMV8_H_ /* prevent circular inclusions */
#define XIL_XARMV8_H_ /* by using protection macros */

/***************************** Include Files ********************************/

#include "xil_types.h"

/**************************** Type Definitions ******************************/
typedef struct {
		u32 TimestampFreq;
		u32 CpuFreq;
		u8 CpuId;	/* CPU Number */
} XARMv8_Config;

#endif /* XIL_XARMV8_H_ */
