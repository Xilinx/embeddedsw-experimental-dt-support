/******************************************************************************
* Copyright (c) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XIL_XARMV8_CONFIG_H_ /* prevent circular inclusions */
#define XIL_XARMV8_CONFIG_H_ /* by using protection macros */

/***************************** Include Files ********************************/

#include "xarmv8.h"

/************************** Variable Definitions ****************************/
extern XARMv8_Config XARMv8_ConfigTable;

/***************** Macros (Inline Functions) Definitions ********************/
#define XGet_TimeStampFreq() XARMv8_ConfigTable.TimestampFreq
#define XGet_CpuFreq() XARMv8_ConfigTable.CpuFreq
#define XGet_CpuId() XARMv8_ConfigTable.CpuId
#endif /* XIL_XARMV8_CONFIG_H_ */
