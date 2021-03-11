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
 * @file ttcps.c
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
/***************************** Include Files *********************************/
#include "xiltimer.h"
#include "xttcps.h"
#include "xinterrupt_wrap.h"

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static u32 XTimer_TtcInit(XTimer *InstancePtr, UINTPTR BaseAddress,
			  XTtcPs *TtcPsInstPtr);
#ifdef XSLEEPTIMER_IS_TTCPS
static void XTimer_TtcModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType);
static void XSleepTimer_TtcStop(XTimer *InstancePtr);
#endif

#ifdef XTICKTIMER_IS_TTCPS
void XTtc_CallbackHandler(void *CallBackRef, u32 StatusEvent);
static void XTimer_TtcTickInterval(XTimer *InstancePtr, u32 Delay);
static void XTimer_TtcIntrHandler(XTimer *InstancePtr);
static void XTickTimer_TtcStop(XTimer *InstancePtr);
static void XTickTimer_SetTtcIntrPriority(XTimer *InstancePtr, u8 Priority);
#endif

#ifdef XSLEEPTIMER_IS_TTCPS
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
	InstancePtr->XTimer_ModifyInterval = XTimer_TtcModifyInterval;
	InstancePtr->XSleepTimer_Stop = XSleepTimer_TtcStop;
	return XST_SUCCESS;
}
#endif

#ifdef XTICKTIMER_IS_TTCPS
u32 XilTickTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_TickIntrHandler = XTimer_TtcIntrHandler;
	InstancePtr->XTimer_TickInterval = XTimer_TtcTickInterval;
	InstancePtr->XTickTimer_Stop = XTickTimer_TtcStop;
	InstancePtr->XTickTimer_SetPriority = XTickTimer_SetTtcIntrPriority;
	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
 * Initialize the TTC Ps Instance
 *
 * @param	InstancePtr is a pointer to the instance to be worked on
 * @param	DeviceId is the IPI Instance to be worked on
 *
 * @return	XST_SUCCESS if initialization was successful
 * 		XST_FAILURE in case of failure
 */
/****************************************************************************/
static u32 XTimer_TtcInit(XTimer *InstancePtr, UINTPTR BaseAddress,
			  XTtcPs *TtcPsInstPtr)
{
	u32 Status = XST_FAILURE;
	XTtcPs_Config *ConfigPtr;

	ConfigPtr = XTtcPs_LookupConfig(BaseAddress);
        if (!ConfigPtr) {
                return Status;
        }

	Status = XTtcPs_CfgInitialize(TtcPsInstPtr, ConfigPtr,
				      ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		XTtcPs_Stop(TtcPsInstPtr);
		Status = XTtcPs_CfgInitialize(TtcPsInstPtr, ConfigPtr,
					      ConfigPtr->BaseAddress);
	}

	XTtcPs_Start(TtcPsInstPtr);

	return Status;
}

#ifdef XTICKTIMER_IS_TTCPS
void XTtc_CallbackHandler(void *CallBackRef, u32 StatusEvent)
{
	XTimer *InstancePtr = (XTimer *)CallBackRef;

	InstancePtr->Handler(InstancePtr->CallBackRef, 0);
}

static void XTimer_TtcTickInterval(XTimer *InstancePtr, u32 Delay)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_TickInst;
	static XInterval Interval;
	static u8 Prescaler;
	u32 Freq;
	static u8 IsTickTimerStarted = FALSE;

	if (FALSE == IsTickTimerStarted) {
#ifdef SDT
		XTimer_TtcInit(InstancePtr, XTICKTIMER_BASEADDRESS,
#else
		XTimer_TtcInit(InstancePtr, XTICKTIMER_DEVICEID,
#endif
				&InstancePtr->TtcPs_TickInst);
		IsTickTimerStarted = TRUE;
	}
	Freq = XTIMER_DELAY_MSEC/Delay;
	XTtcPs_SetOptions(TtcPsInstPtr, XTTCPS_OPTION_INTERVAL_MODE |
			XTTCPS_OPTION_WAVE_DISABLE);
	XTtcPs_CalcIntervalFromFreq(TtcPsInstPtr, Freq, &Interval, &Prescaler);
	XTtcPs_SetInterval(TtcPsInstPtr, Interval);
	XTtcPs_SetPrescaler(TtcPsInstPtr, Prescaler);
	XTtcPs_EnableInterrupts(TtcPsInstPtr, XTTCPS_IXR_INTERVAL_MASK);
        XTtcPs_Start(TtcPsInstPtr);
}

static void XTimer_TtcIntrHandler(XTimer *InstancePtr)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_TickInst;

	XTtcPs_SetStatusHandler(TtcPsInstPtr, InstancePtr,
		              (XTtcPs_StatusHandler)XTtc_CallbackHandler);
#ifndef SDT
	XSetupInterruptSystem(TtcPsInstPtr, XTtcPs_InterruptHandler,
			      TtcPsInstPtr->Config.IntrId,
			      TtcPsInstPtr->Config.IntrParent,
			      XINTERRUPT_DEFAULT_PRIORITY);
#else
	XSetupInterruptSystem(TtcPsInstPtr, XTtcPs_InterruptHandler,
			      TtcPsInstPtr->Config.IntrId[0],
			      TtcPsInstPtr->Config.IntrParent,
			      XINTERRUPT_DEFAULT_PRIORITY);
#endif
}

static void XTickTimer_TtcStop(XTimer *InstancePtr)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_TickInst;

	XTtcPs_Stop(TtcPsInstPtr);
}

static void XTickTimer_SetTtcIntrPriority(XTimer *InstancePtr, u8 Priority)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_TickInst;

#ifndef SDT
	XSetPriorityTriggerType(TtcPsInstPtr->Config.IntrId, Priority,
				TtcPsInstPtr->Config.IntrParent);
#else
	XSetPriorityTriggerType(TtcPsInstPtr->Config.IntrId[0], Priority,
				TtcPsInstPtr->Config.IntrParent);
#endif
}
#endif

#ifdef XSLEEPTIMER_IS_TTCPS
static void XTimer_TtcModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_SleepInst;
	u64 tEnd = 0U;
	u64 tCur = 0U;
	u32 TimeHighVal = 0U;
	u32 TimeLowVal1 = 0U;
	u32 TimeLowVal2 = 0U;
	static u8 IsSleepTimerStarted = FALSE;

	if (FALSE == IsSleepTimerStarted) {
#ifdef SDT
		XTimer_TtcInit(InstancePtr, XSLEEPTIMER_BASEADDRESS,
#else
		XTimer_TtcInit(InstancePtr, XSLEEPTIMER_DEVICEID,
#endif
				&InstancePtr->TtcPs_SleepInst);
		IsSleepTimerStarted = TRUE;
	}

	TimeLowVal1 = XTtcPs_GetCounterValue(TtcPsInstPtr);
	tEnd = (u64)TimeLowVal1 + ((u64)(delay) *
                                   TtcPsInstPtr->Config.InputClockHz / (DelayType));
	do {
		TimeLowVal2 = XTtcPs_GetCounterValue(TtcPsInstPtr);
		if (TimeLowVal2 < TimeLowVal1) {
			TimeHighVal++;
		}
		TimeLowVal1 = TimeLowVal2;
		tCur = (((u64) TimeHighVal) << 32U) | (u64)TimeLowVal2;
	} while (tCur < tEnd);
}

static void XSleepTimer_TtcStop(XTimer *InstancePtr)
{
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_SleepInst;

	XTtcPs_Stop(TtcPsInstPtr);
}

void XTime_GetTime(XTime *Xtime_Global)
{
	XTimer *InstancePtr = &TimerInst;
	XTtcPs *TtcPsInstPtr = &InstancePtr->TtcPs_SleepInst;

	*Xtime_Global = XTtcPs_GetCounterValue(TtcPsInstPtr);
}
#endif
