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
 * @file cortexr5_sleep.c
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
#include "xpm_counter.h"

#ifdef XTIMER_IS_DEFAULT_TIMER 
#include "xcortexr5_config.h"
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static void XCortexr5_ModifyInterval(XTimer *InstancePtr, u32 delay,
				       XTimer_DelayType DelayType);

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
	InstancePtr->XTimer_ModifyInterval = XCortexr5_ModifyInterval;
	InstancePtr->XSleepTimer_Stop = NULL;

	return XST_SUCCESS;
}

static void XCortexr5_ModifyInterval(XTimer *InstancePtr, u32 delay,
				       XTimer_DelayType DelayType)
{
	u64 tEnd = 0U;
	u64 tCur = 0U;
	u32 TimeHighVal = 0U;
	u32 TimeLowVal1 = 0U;
	u32 TimeLowVal2 = 0U;
	/* For the CortexR5 PMU cycle counter. As boot code is setting up "D"
	 * bit in PMCR, cycle counter increments on every 64th bit of processor cycle
	 */
	u32 frequency = XGet_CpuFreq()/64;

#if defined (__GNUC__)
	TimeLowVal1 = Xpm_ReadCycleCounterVal();
#elif defined (__ICCARM__)
	Xpm_ReadCycleCounterVal(TimeLowVal1);
#endif

	tEnd = (u64)TimeLowVal1 + ((u64)(delay) * (frequency/(DelayType)));

	do {
#if defined (__GNUC__)
		TimeLowVal2 = Xpm_ReadCycleCounterVal();
#elif defined (__ICCARM__)
		Xpm_ReadCycleCounterVal(TimeLowVal2);
#endif
		if (TimeLowVal2 < TimeLowVal1) {
			TimeHighVal++;
		}
		TimeLowVal1 = TimeLowVal2;
		tCur = (((u64) TimeHighVal) << 32) | (u64)TimeLowVal2;
	} while (tCur < tEnd);
}

void XTime_GetTime(XTime *Xtime_Global) {
	XTimer *InstancePtr = &TimerInst;

	*Xtime_Global = Xpm_ReadCycleCounterVal();
}
#endif

#ifdef XTIMER_NO_TICK_TIMER
u32 XilTickTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_TickIntrHandler = NULL;
	InstancePtr->XTimer_TickInterval = NULL;
	InstancePtr->XTickTimer_Stop = NULL;
	return XST_SUCCESS;
}
#endif
