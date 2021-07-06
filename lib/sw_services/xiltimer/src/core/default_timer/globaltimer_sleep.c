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
#ifdef SDT
#include "xarmv8_config.h"
#endif

/**************************** Type Definitions *******************************/
/************************** Constant Definitions *****************************/
#if defined (versal)
#define XIOU_SCNTRS_BASEADDR	0xFF140000U
#else
#define XIOU_SCNTRS_BASEADDR	0xFF260000U
#endif
#define XIOU_SCNTRS_CNT_CNTRL_REG_OFFSET	0x00000000U
#define XIOU_SCNTRS_FREQ_REG_OFFSET		0x00000020U
#define XIOU_SCNTRS_CNT_CNTRL_REG_EN            0x00000001U
#define XIOU_SCNTRS_CNT_CNTRL_REG_EN_MASK	0x00000001U

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

#ifdef SDT
	u32 TimerStampFreq = XGet_TimeStampFreq();
	mtcp(CNTFRQ_EL0, TimerStampFreq);
#endif

	return XST_SUCCESS;
}

static void XGlobalTimer_Start(XTimer *InstancePtr)
{
	(void) InstancePtr;
#ifndef SDT
        u32 TimerStampFreq = XPAR_CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ;
#else
        u32 TimerStampFreq = XGet_TimeStampFreq();
#endif

	if (EL3 == 1){
                /* Enable the global timer counter only if it is disabled */
                if(((Xil_In32(XIOU_SCNTRS_BASEADDR + XIOU_SCNTRS_CNT_CNTRL_REG_OFFSET))
                                        & XIOU_SCNTRS_CNT_CNTRL_REG_EN_MASK) !=
                                        XIOU_SCNTRS_CNT_CNTRL_REG_EN){
                        /*write frequency to System Time Stamp Generator Register*/
                        Xil_Out32((XIOU_SCNTRS_BASEADDR + XIOU_SCNTRS_FREQ_REG_OFFSET),
                                   TimerStampFreq);
                        /*Enable the timer/counter*/
                        Xil_Out32((XIOU_SCNTRS_BASEADDR + XIOU_SCNTRS_CNT_CNTRL_REG_OFFSET)
                                                ,XIOU_SCNTRS_CNT_CNTRL_REG_EN);
                }
        }
}

static void XGlobalTimer_ModifyInterval(XTimer *InstancePtr, u32 delay,
					XTimer_DelayType DelayType)
{
	(void) InstancePtr;
	XTime tEnd, tCur;
#ifndef SDT
        u32 TimerStampFreq = XPAR_CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ;
#else
        u32 TimerStampFreq = XGet_TimeStampFreq();
#endif
        u32 iterpersec = TimerStampFreq;
	static u8 IsSleepTimerStarted = FALSE;

	if (FALSE == IsSleepTimerStarted) {
		XGlobalTimer_Start(InstancePtr);
		IsSleepTimerStarted = TRUE;
	}
	tCur = mfcp(CNTPCT_EL0);
	tEnd = tCur + (((XTime) delay) * (iterpersec / DelayType));
        do {
                tCur = mfcp(CNTPCT_EL0);
        } while (tCur < tEnd);

}

void XTime_GetTime(XTime *Xtime_Global)
{
	*Xtime_Global = mfcp(CNTPCT_EL0);
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
