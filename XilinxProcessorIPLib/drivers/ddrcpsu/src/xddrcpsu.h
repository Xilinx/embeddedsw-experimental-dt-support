/*******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xddcrpsu.h
 * @addtogroup ddrcpsu_v1_3
 * @{
 * @details
 *
 * The Xilinx DdrcPsu driver. This driver supports the Xilinx ddrcpsu
 * IP core.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0	ssc   04/28/16 First Release.
 * 1.1  adk   04/08/16 Export DDR freq to xparameters.h file.
 *
 * </pre>
 *
*******************************************************************************/

#ifndef XDDRCPS_H_
/* Prevent circular inclusions by using protection macros. */
#define XDDRCPS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/******************************* Include Files ********************************/

typedef struct {
	char *Name;
	UINTPTR BaseAddress;
	u8 HasEccEn;
	u32 InputClockFreq;
	u8 AddrMapping;
	u32 DdrFreq; /* DDR Freq in Hz */
	u32 VideoBufSize;
	u32 BrcMapping;
	u8 HasDynamicDDrEn;
	u8 Memtype;
	u8 MemAddrMap;
	u32 DataMask_Dbi;
	u32 AddressMirror;
	u32 Secondclk;
	u32 Parity;
	u32 PwrDnEn;
	u32 ClockStopEn;
        u32 LpAsr;
	u32 TRefMode;
	u32 Fgrm;
	u32 SelfRefAbort;
	u32 TRefRange;
} XDdrcpsu_Config;


typedef struct {
	XDdrcpsu_Config Config;	/**< Configuration structure */
	u32 IsReady;		/**< Device is initialized and ready */
} XDdrcPsu;

XDdrcpsu_Config *XDdrcPsu_LookupConfig(u32 BaseAddress);
s32 XDdrcPsu_CfgInitialize(XDdrcPsu *InstancePtr, XDdrcpsu_Config *CfgPtr);
#ifdef __cplusplus
}

#endif

#endif /* XDDRCPS_H_ */
/** @} */
