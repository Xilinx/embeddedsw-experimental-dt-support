/******************************************************************************
 *
 * Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
* @file xiltimer.h
*
*  This header file contains.
*
* <pre>
* MODIFICATION HISTORY :
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -------------------------------------------------------
*
* </pre>
*
******************************************************************************/

#ifndef XILTIMER_H
#define XILTIMER_H

#include "xtimer_config.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "bspconfig.h"
#if defined(XSLEEPTIMER_IS_AXITIMER) || defined(XTICKTIMER_IS_AXITIMER)
#include "xtmrctr.h"
#endif
#if defined(XSLEEPTIMER_IS_TTCPS) || defined(XTICKTIMER_IS_TTCPS)
#include "xttcps.h"
#endif
#if defined(XSLEEPTIMER_IS_SCUTIMER) || defined(XTICKTIMER_IS_SCUTIMER)
#include "xscutimer.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	XTIMER_DELAY_SEC = 1,
	XTIMER_DELAY_MSEC = 1000,
	XTIMER_DELAY_USEC = 1000 * 1000,
} XTimer_DelayType;

typedef void (*XTimer_TickHandler) (void *CallBackRef, u32 StatusEvent);

typedef struct XTimerTag {
	void (*XTimer_ModifyInterval)(struct XTimerTag *InstancePtr, u32 delay, XTimer_DelayType Delaytype);
	void (*XTimer_TickIntrHandler)(struct XTimerTag *InstancePtr);
	void (*XTimer_TickInterval)(struct XTimerTag *InstancePtr, u32 Delay);
	void (*XSleepTimer_Stop)(struct XTimerTag *InstancePtr);
	void (*XTickTimer_Stop)(struct XTimerTag *InstancePtr);
	void (*XTickTimer_SetPriority)(struct XTimerTag *InstancePtr, u8 Priority);
	XTimer_TickHandler Handler; /**< Callback function */
	void *CallBackRef;       /**< Callback reference for handler */
#ifdef XSLEEPTIMER_IS_AXITIMER
	XTmrCtr AxiTimer_SleepInst;
#endif
#ifdef XTICKTIMER_IS_AXITIMER
	XTmrCtr AxiTimer_TickInst;
#endif
#ifdef XSLEEPTIMER_IS_TTCPS
	XTtcPs TtcPs_SleepInst;
#endif
#ifdef XTICKTIMER_IS_TTCPS
	XTtcPs TtcPs_TickInst;
#endif
#ifdef XSLEEPTIMER_IS_SCUTIMER
	XScuTimer ScuTimer_SleepInst;
#endif
#ifdef XTICKTIMER_IS_SCUTIMER
	XScuTimer ScuTimer_TickInst;
#endif
} XTimer;

typedef u64 XTime;
static XTimer TimerInst;

u32 XilSleepTimer_Init(XTimer *InstancePtr);
u32 XilTickTimer_Init(XTimer *InstancePtr);
void XTime_GetTime(XTime *Xtime_Global);
void XTimer_SetInterval(unsigned long delay);
void XTimer_SetHandler(XTimer_TickHandler FuncPtr, void *CallBackRef);
void XTimer_SetTickPriority(u8 Priority);

#ifdef __cplusplus
}
#endif

#endif
