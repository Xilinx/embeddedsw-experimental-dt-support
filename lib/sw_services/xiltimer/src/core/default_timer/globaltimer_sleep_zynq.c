/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file globaltimer_sleep.c
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

#ifdef XTIMER_IS_DEFAULT_TIMER

#include "xcortexa9_config.h"

/**************************** Type Definitions *******************************/
/************************** Constant Definitions *****************************/
#define	GLOBAL_TMR_BASEADDR	0xF8F00200U
#define GTIMER_COUNTER_LOWER_OFFSET	0x00U
#define GTIMER_COUNTER_UPPER_OFFSET	0x04U
#define GTIMER_CONTROL_OFFSET	0x08U

/************************** Function Prototypes ******************************/
static void XGlobalTimer_Start(XTimer *InstancePtr);
static void XGlobalTimer_ModifyInterval(XTimer *InstancePtr, u32 delay,
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
	InstancePtr->XTimer_ModifyInterval = XGlobalTimer_ModifyInterval;
	InstancePtr->XSleepTimer_Stop = NULL;

	return XST_SUCCESS;
}

static void XGlobalTimer_Start(XTimer *InstancePtr)
{
	(void) InstancePtr;

	/* Disable Global Timer */
	Xil_Out32((u32)GLOBAL_TMR_BASEADDR + (u32)GTIMER_CONTROL_OFFSET, (u32)0x0);

	/* Updating Global Timer Counter Register */
	Xil_Out32((u32)GLOBAL_TMR_BASEADDR + (u32)GTIMER_COUNTER_LOWER_OFFSET, 0x0);
	Xil_Out32((u32)GLOBAL_TMR_BASEADDR + (u32)GTIMER_COUNTER_UPPER_OFFSET,
		0x0);

	/* Enable Global Timer */
	Xil_Out32((u32)GLOBAL_TMR_BASEADDR + (u32)GTIMER_CONTROL_OFFSET, (u32)0x1);
}

static void XGlobalTimer_ModifyInterval(XTimer *InstancePtr, u32 delay,
					XTimer_DelayType DelayType)
{
	(void) InstancePtr;
	XTime tEnd, tCur;
        u32 CpuFreq = XGet_CpuFreq();

        /* Global Timer is always clocked at half of the CPU frequency */
        u32 TimerCountsPersec = CpuFreq/2;
	static u8 IsSleepTimerStarted = FALSE;

	if (FALSE == IsSleepTimerStarted) {
		XGlobalTimer_Start(InstancePtr);
		IsSleepTimerStarted = TRUE;
	}

	XTime_GetTime(&tCur);
	tEnd = tCur + (((XTime) delay) * (TimerCountsPersec / DelayType));
        do {
		XTime_GetTime(&tCur);
        } while (tCur < tEnd);

}

void XTime_GetTime(XTime *Xtime_Global)
{
	u32 low;
	u32 high;

	/* Reading Global Timer Counter Register */
	do
	{
		high = Xil_In32(GLOBAL_TMR_BASEADDR + GTIMER_COUNTER_UPPER_OFFSET);
		low = Xil_In32(GLOBAL_TMR_BASEADDR + GTIMER_COUNTER_LOWER_OFFSET);
	} while(Xil_In32(GLOBAL_TMR_BASEADDR + GTIMER_COUNTER_UPPER_OFFSET) != high);

	*Xtime_Global = (((XTime) high) << 32U) | (XTime) low;
}

#endif /* XTIMER_IS_DEFAULT_TIMER */

#ifdef XTIMER_NO_TICK_TIMER
u32 XilTickTimer_Init(XTimer *InstancePtr)
{
	InstancePtr->XTimer_TickIntrHandler = NULL;
	InstancePtr->XTimer_TickInterval = NULL;
	InstancePtr->XTickTimer_Stop = NULL;
	return XST_SUCCESS;
}
#endif
