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
 *
 * @file axi_timer.c
 * @addtogroup xiltimer_v1_0
 * @{
 * @details
 *
 * This file contains the definitions for axi timer implementation.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   adk 
 *</pre>
 *
 *@note
 *****************************************************************************/
#include "xiltimer.h"

/***************************** Include Files *********************************/
#include "xtmrctr.h"
#include "xinterrupt_wrap.h"

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static u32 XAxiTimer_Init(XTimer *InstancePtr, UINTPTR BaseAddress,
			  XTmrCtr *AxiTimerInstPtr);
#ifdef XSLEEPTIMER_IS_AXITIMER
static void XAxiTimer_ModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType);
static void XSleepTimer_AxiTimerStop(XTimer *InstancePtr);
#endif

#ifdef XTICKTIMER_IS_AXITIMER
static void XAxiTimer_TickInterval(XTimer *InstancePtr, u32 Delay);
static void XAxiTimer_IntrHandler(XTimer *InstancePtr);
void XAxiTimer_CallbackHandler(void *CallBackRef, u8 TmrCtrNumber);
static void XTickTimer_AxiTimerStop(XTimer *InstancePtr);
#endif

#ifdef XSLEEPTIMER_IS_AXITIMER
/****************************************************************************/
/**
 * Initialize the XTimer Instance
 *
 * @param	InstancePtr is a pointer to the instance to be worked on
 *
 * @return	XST_SUCCESS if initialization was successful
 * 		XST_FAILURE in case of failure
 */
/****************************************************************************/
u32 XilSleepTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_ModifyInterval = XAxiTimer_ModifyInterval;
	InstancePtr->XSleepTimer_Stop = XSleepTimer_AxiTimerStop;

	return XST_SUCCESS;
}
#endif

#ifdef XTICKTIMER_IS_AXITIMER
u32 XilTickTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_TickIntrHandler = XAxiTimer_IntrHandler;
	InstancePtr->XTimer_TickInterval = XAxiTimer_TickInterval;
	InstancePtr->XTickTimer_Stop = XTickTimer_AxiTimerStop;

	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
 * Initialize the Axi Timer Instance
 *
 * @param	InstancePtr is a pointer to the instance to be worked on
 * @param	DeviceId is the IPI Instance to be worked on
 *
 * @return	XST_SUCCESS if initialization was successful
 * 		XST_FAILURE in case of failure
 */
/****************************************************************************/
static u32 XAxiTimer_Init(XTimer *InstancePtr, UINTPTR BaseAddress,
			  XTmrCtr *AxiTimerInstPtr)
{
	u32 Status = XST_FAILURE;
	XTmrCtr_Config *ConfigPtr;
	
	ConfigPtr = XTmrCtr_LookupConfig(BaseAddress);
	if (!ConfigPtr) {
		return Status;
	}

	XTmrCtr_CfgInitialize(AxiTimerInstPtr, ConfigPtr, ConfigPtr->BaseAddress);
	XTmrCtr_InitHw(AxiTimerInstPtr);
	XTmrCtr_SetOptions(AxiTimerInstPtr, 0,
			   XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_Start(AxiTimerInstPtr, 0);

	return XST_SUCCESS; 
}

#ifdef XTICKTIMER_IS_AXITIMER
void XAxiTimer_CallbackHandler(void *CallBackRef, u8 TmrCtrNumber)
{
	XTimer *InstancePtr = (XTimer *)CallBackRef;

	InstancePtr->Handler(InstancePtr->CallBackRef, 0);
}

static void XAxiTimer_TickInterval(XTimer *InstancePtr, u32 Delay)
{
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_TickInst;
	u32 Tlr;
	static u8 IsTickTimerStarted = FALSE;

	if (FALSE == IsTickTimerStarted) {
#ifdef SDT
		XAxiTimer_Init(InstancePtr, XTICKTIMER_BASEADDRESS,
#else
		XAxiTimer_Init(InstancePtr, XTICKTIMER_DEVICEID,
#endif
				&InstancePtr->AxiTimer_TickInst);
		IsTickTimerStarted = TRUE;
	}

	Tlr = Delay * (AxiTimerInstPtr->Config.SysClockFreqHz /
			XTIMER_DELAY_MSEC);
	XTmrCtr_SetOptions(AxiTimerInstPtr, 0,
                           XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION |
			   XTC_DOWN_COUNT_OPTION);

        /*
         * Set a reset value for the timer counter such that it will expire
         * eariler than letting it roll over from 0, the reset value is loaded
         * into the timer counter when it is started
         */
        XTmrCtr_SetResetValue(AxiTimerInstPtr, 0, Tlr);

        /*
         * Start the timer counter such that it's incrementing by default,
         * then wait for it to timeout a number of times
         */
        XTmrCtr_Start(AxiTimerInstPtr, 0);
}

static void XAxiTimer_IntrHandler(XTimer *InstancePtr)
{
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_TickInst;

	XTmrCtr_SetHandler(AxiTimerInstPtr, XAxiTimer_CallbackHandler,
			   InstancePtr);
	XSetupInterruptSystem(AxiTimerInstPtr, XTmrCtr_InterruptHandler,
			      AxiTimerInstPtr->Config.IntrId,
			      AxiTimerInstPtr->Config.IntrParent,
			      XINTERRUPT_DEFAULT_PRIORITY);
}

static void XTickTimer_AxiTimerStop(XTimer *InstancePtr)
{
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_TickInst;

	XTmrCtr_Stop(AxiTimerInstPtr, 0);
}
#endif

#ifdef XSLEEPTIMER_IS_AXITIMER
static void XAxiTimer_ModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType)
{
	u32 Status = XST_FAILURE;
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_SleepInst;
	u64 tEnd = 0U;
	u64 tCur = 0U;
	u32 TimeHighVal = 0U;
	u32 TimeLowVal1 = 0U;
	u32 TimeLowVal2 = 0U;
	static u8 IsSleepTimerStarted = FALSE;

	if (FALSE == IsSleepTimerStarted) {
#ifdef SDT
		XAxiTimer_Init(InstancePtr, XSLEEPTIMER_BASEADDRESS,
#else
		XAxiTimer_Init(InstancePtr, XSLEEPTIMER_DEVICEID,
#endif
				&InstancePtr->AxiTimer_SleepInst);
		IsSleepTimerStarted = TRUE;
	}

	TimeLowVal1 = XTmrCtr_GetValue(AxiTimerInstPtr, 0);
	tEnd = (u64)TimeLowVal1 + ((u64)(delay) *
			AxiTimerInstPtr->Config.SysClockFreqHz / (DelayType));
	do {
		TimeLowVal2 = XTmrCtr_GetValue(AxiTimerInstPtr, 0);
		if (TimeLowVal2 < TimeLowVal1) {
			TimeHighVal++;
		}
		TimeLowVal1 = TimeLowVal2;
		tCur = (((u64) TimeHighVal) << 32U) | (u64)TimeLowVal2;
	} while (tCur < tEnd);
}

static void XSleepTimer_AxiTimerStop(XTimer *InstancePtr)
{
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_SleepInst;

	XTmrCtr_Stop(AxiTimerInstPtr, 0);
}

void XTime_GetTime(XTime *Xtime_Global)
{
	XTimer *InstancePtr = &TimerInst;
	XTmrCtr *AxiTimerInstPtr = &InstancePtr->AxiTimer_SleepInst;

	*Xtime_Global = XTmrCtr_GetValue(AxiTimerInstPtr, 0);
}
#endif
