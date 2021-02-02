/******************************************************************************
* Copyright (c) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XIL_XCORTEXR5_H_ /* prevent circular inclusions */
#define XIL_XCORTEXR5_H_ /* by using protection macros */

/***************************** Include Files ********************************/

#include "xil_types.h"

/**************************** Type Definitions ******************************/
typedef struct {
		u32 CpuFreq;
		u8 CpuId;	/* CPU Number */
} XCortexr5_Config;

#endif /* XIL_XARMV8_H_ */
