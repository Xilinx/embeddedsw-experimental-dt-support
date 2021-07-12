/******************************************************************************
* Copyright (C) 2005 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XAXICDMA_SG_INTR_HEADER_H		/* prevent circular inclusions */
#define XAXICDMA_SG_INTR_HEADER_H		/* by using protection macros */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifndef SDT
int XAxiCdma_SgIntrExample(XAxiCdma *InstancePtr, u16 DeviceId);
#else
int XAxiCdma_SgIntrExample(XAxiCdma *InstancePtr, UINTPTR BaseAddress);
#endif

