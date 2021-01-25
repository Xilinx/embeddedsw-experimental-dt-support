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
 * @file scutimer.c
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
#include "xscutimer.h"
#include "xinterrupt_wrap.h"

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static u32 XTimer_ScutimerInit(XTimer *InstancePtr, UINTPTR BaseAddress,
			       XScuTimer *ScuTimerInstPtr);
#ifdef XSLEEPTIMER_IS_SCUTIMER
static void XTimer_ScutimerModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType);
static void XSleepTimer_ScutimerStop(XTimer *InstancePtr);
#endif

#ifdef XTICKTIMER_IS_SCUTIMER
void XScutimer_CallbackHandler(void *CallBackRef);
static void XTimer_ScutimerTickInterval(XTimer *InstancePtr, u32 Delay);
static void XTimer_ScutimerIntrHandler(XTimer *InstancePtr);
static void XTickTimer_ScutimerStop(XTimer *InstancePtr);
static void XTickTimer_SetScutimerIntrPriority(XTimer *InstancePtr, u8 Priority);
#endif

#ifdef XSLEEPTIMER_IS_SCUTIMER
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
	InstancePtr->XTimer_ModifyInterval = XTimer_ScutimerModifyInterval;
	InstancePtr->XSleepTimer_Stop = XSleepTimer_ScutimerStop;
	return XST_SUCCESS;
}
#endif

#ifdef XTICKTIMER_IS_SCUTIMER
u32 XilTickTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_TickIntrHandler = XTimer_ScutimerIntrHandler;
	InstancePtr->XTimer_TickInterval = XTimer_ScutimerTickInterval;
	InstancePtr->XTickTimer_Stop = XTickTimer_ScutimerStop;
	InstancePtr->XTickTimer_SetPriority = XTickTimer_SetScutimerIntrPriority;
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
static u32 XTimer_ScutimerInit(XTimer *InstancePtr, UINTPTR BaseAddress,
			       XScuTimer *ScuTimerInstPtr)
{
	u32 Status = XST_FAILURE;
	XScutimer_Config *ConfigPtr;

	ConfigPtr = XScuTimer_LookupConfig(BaseAddress);
        if (!ConfigPtr) {
                return Status;
        }

	xil_printf("ConfigPtr->BaseAddr is %x\n\r", ConfigPtr->BaseAddr);
	xil_printf("ConfigPtr->IntrId is %x and %x\n\r", ConfigPtr->IntrId, ConfigPtr->IntrParent);
	Status = XScuTimer_CfgInitialize(ScuTimerInstPtr, ConfigPtr,
					 ConfigPtr->BaseAddr);
	XScuTimer_EnableAutoReload(ScuTimerInstPtr);
	XScuTimer_Start(ScuTimerInstPtr);

	return Status;
}

#ifdef XTICKTIMER_IS_SCUTIMER
void XScutimer_CallbackHandler(void *CallBackRef)
{
	XTimer *InstancePtr = (XTimer *)CallBackRef;
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_TickInst;

	XScuTimer_ClearInterruptStatus(ScuTimerInstPtr);
	InstancePtr->Handler(InstancePtr->CallBackRef, 0);
}

static void XTimer_ScutimerTickInterval(XTimer *InstancePtr, u32 Delay)
{
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_TickInst;
	static XInterval Interval;
	static u8 Prescaler;
	u32 Freq;
	static u8 IsTickTimerStarted = FALSE;
	u32 ScuTimerFreq = 666666687 / 2U;

	if (FALSE == IsTickTimerStarted) {
		xil_printf("XTICKTIMER_BASEADDRESS is %x\n\r", XTICKTIMER_BASEADDRESS);
		XTimer_ScutimerInit(InstancePtr, XTICKTIMER_BASEADDRESS,
				    &InstancePtr->ScuTimer_TickInst);
		IsTickTimerStarted = TRUE;
	}
	Freq = XTIMER_DELAY_MSEC/Delay;
	xil_printf("load value is %x\n\r", ScuTimerFreq/Freq);
	XScuTimer_Stop(ScuTimerInstPtr);
	XScuTimer_EnableAutoReload(ScuTimerInstPtr);
	XScuTimer_SetPrescaler(ScuTimerInstPtr, 0);
	XScuTimer_LoadTimer(ScuTimerInstPtr, ScuTimerFreq/Freq);
	XScuTimer_EnableInterrupt(ScuTimerInstPtr);
	XScuTimer_Start(ScuTimerInstPtr);
}

static void XTimer_ScutimerIntrHandler(XTimer *InstancePtr)
{
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_TickInst;

	xil_printf("Base address is %x\n\r", ScuTimerInstPtr->Config.BaseAddr);
	xil_printf("IntrId is %x\n\r", ScuTimerInstPtr->Config.IntrId);
	xil_printf("GIC base addr is %x\n\r", ScuTimerInstPtr->Config.IntrParent);
	XSetupInterruptSystem(InstancePtr, XScutimer_CallbackHandler,
			      ScuTimerInstPtr->Config.IntrId,
			      ScuTimerInstPtr->Config.IntrParent,
			      XINTERRUPT_DEFAULT_PRIORITY);
}

static void XTickTimer_ScutimerStop(XTimer *InstancePtr)
{
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_TickInst;

	XScuTimer_Stop(ScuTimerInstPtr);
}

static void XTickTimer_SetScutimerIntrPriority(XTimer *InstancePtr, u8 Priority)
{
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_TickInst;

	XSetPriorityTriggerType(ScuTimerInstPtr->Config.IntrId, Priority,
				ScuTimerInstPtr->Config.IntrParent);
}
#endif

#ifdef XSLEEPTIMER_IS_SCUTIMER
static void XTimer_ScutimerModifyInterval(XTimer *InstancePtr, u32 delay,
				     XTimer_DelayType DelayType)
{
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_SleepInst;
	u64 tEnd = 0U;
	u64 tCur = 0U;
	u32 TimeHighVal = 0U;
	u32 TimeLowVal1 = 0U;
	u32 TimeLowVal2 = 0U;
	static u8 IsSleepTimerStarted = FALSE;
	//u32 ScuTimerFreq = XGet_CpuFreq() / 2U;
	u32 ScuTimerFreq = 666666687 / 2U;

	if (FALSE == IsSleepTimerStarted) {
		XTimer_ScutimerInit(InstancePtr, XSLEEPTIMER_BASEADDRESS,
				    &InstancePtr->ScuTimer_SleepInst);
		IsSleepTimerStarted = TRUE;
	}

	TimeLowVal1 = XScuTimer_GetCounterValue(ScuTimerInstPtr);
	tEnd = (u64)TimeLowVal1 + ((u64)(delay) *
                                   ScuTimerFreq / (DelayType));
	do {
		TimeLowVal2 = XScuTimer_GetCounterValue(ScuTimerInstPtr);
		if (TimeLowVal2 < TimeLowVal1) {
			TimeHighVal++;
		}
		TimeLowVal1 = TimeLowVal2;
		tCur = (((u64) TimeHighVal) << 32U) | (u64)TimeLowVal2;
	} while (tCur < tEnd);
}

static void XSleepTimer_ScutimerStop(XTimer *InstancePtr)
{
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_SleepInst;

	XScuTimer_Stop(ScuTimerInstPtr);
}

void XTime_GetTime(XTime *Xtime_Global)
{
	XTimer *InstancePtr = &TimerInst;
	XScuTimer *ScuTimerInstPtr = &InstancePtr->ScuTimer_SleepInst;

	*Xtime_Global = XScuTimer_GetCounterValue(ScuTimerInstPtr);
}
#endif
