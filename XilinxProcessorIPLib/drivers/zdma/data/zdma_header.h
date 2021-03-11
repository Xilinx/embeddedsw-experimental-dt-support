/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef ZDMA_HEADER_H		/* prevent circular inclusions */
#define ZDMA_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int XZDma_SelfTestExample(u16 DeviceId);
#if defined(XPAR_SCUGIC_0_DEVICE_ID) || defined(XPAR_INTC_0_DEVICE_ID)
int XZDma_SimpleExample(XZDma *ZdmaInstPtr,
			u16 DeviceId);
#endif
#else
int XZDma_SelfTestExample(UINTPTR BaseAddress);
int XZDma_SimpleExample(XZDma *ZdmaInstPtr,
			UINTPTR BaseAddress);
#endif
#endif
