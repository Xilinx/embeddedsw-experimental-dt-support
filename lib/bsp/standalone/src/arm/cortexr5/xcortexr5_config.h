/******************************************************************************
* Copyright (c) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XIL_XARMV8_CONFIG_H_ /* prevent circular inclusions */
#define XIL_XARMV8_CONFIG_H_ /* by using protection macros */

/***************************** Include Files ********************************/

#include "xcortexr5.h"

/************************** Variable Definitions ****************************/
extern XCortexr5_Config XCortexr5_ConfigTable;

/***************** Macros (Inline Functions) Definitions ********************/
#define XGet_CpuFreq() XCortexr5_ConfigTable.CpuFreq
#define XGet_CpuId() XCortexr5_ConfigTable.CpuId
#endif /* XIL_XARMV8_CONFIG_H_ */
