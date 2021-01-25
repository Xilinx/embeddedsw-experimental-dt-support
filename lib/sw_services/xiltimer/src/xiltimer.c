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
*@file xil_timer.c
*
* This file contains the sleep API's
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* </pre>
******************************************************************************/


/***************************** Include Files *********************************/
#include "xil_io.h"
#include "sleep.h"
#include "xiltimer.h"

/****************************  Constant Definitions  *************************/
void XilTimer_Sleep(unsigned long delay, XTimer_DelayType DelayType);

/*****************************************************************************/
/**
*
* This API gives delay in sec
*
* @param            seconds - delay time in seconds
*
* @return           none
*
* @note             none
*
*****************************************************************************/
void sleep(unsigned int seconds) {
	XilTimer_Sleep(seconds, XTIMER_DELAY_SEC);
}

/****************************************************************************/
/**
*
* This API gives delay in msec
*
* @param            mseconds - delay time in mseconds
*
* @return           none
*
* @note             none
*
*****************************************************************************/
void msleep(unsigned long mseconds) {
	XilTimer_Sleep(mseconds, XTIMER_DELAY_MSEC);
}

/****************************************************************************/
/**
*
* This API gives delay in usec
*
* @param            useconds - delay time in useconds
*
* @return           none
*
* @note             none
*
*****************************************************************************/
void usleep(unsigned long useconds) {
	XilTimer_Sleep(useconds, XTIMER_DELAY_USEC);
}

/****************************************************************************/
/**
*
* This API contains common delay implementation using library API's.
*
* @param            useconds - delay time in useconds
*
* @return           none
*
* @note             none
*
*****************************************************************************/
void XilTimer_Sleep(unsigned long delay, XTimer_DelayType DelayType) {
	XTimer *InstancePtr;

	InstancePtr = &TimerInst;
	if (InstancePtr->XTimer_ModifyInterval)
		InstancePtr->XTimer_ModifyInterval(InstancePtr, delay,
						   DelayType);
}

void __attribute__ ((constructor)) xtimerinit()
{
    
    xil_printf("inside %s\n\r", __func__);
    XilSleepTimer_Init(&TimerInst);
    XilTickTimer_Init(&TimerInst);
}

/****************************************************************************/
/**
*
* This routine installs an asynchronous callback function for the given
* FuncPtr.
*
* @param	FuncPtr is the address of the callback function.
* @param	CallBackRef is a user data item that will be passed to the
* 		callback function when it is invoked.
*
* @return	None
*
*****************************************************************************/
void XTimer_SetHandler(XTimer_TickHandler FuncPtr, void *CallBackRef)
{
	XTimer *InstancePtr;

	InstancePtr = &TimerInst;

	InstancePtr->Handler = FuncPtr;
	InstancePtr->CallBackRef = CallBackRef;
	if (InstancePtr->XTimer_TickIntrHandler) {
		InstancePtr->XTimer_TickIntrHandler(InstancePtr);
	}
}

/****************************************************************************/
/**
*
* This API sets the elapse interval for the timer instance.
*
* @param            delay - delay time in milli seconds
*
* @return           none
*
* @note             none
*
*****************************************************************************/
void XTimer_SetInterval(unsigned long delay)
{
	XTimer *InstancePtr;

	InstancePtr = &TimerInst;
	if (InstancePtr->XTimer_TickInterval)
		InstancePtr->XTimer_TickInterval(InstancePtr, delay);
}

void XTimer_SetTickPriority(u8 Priority)
{
	XTimer *InstancePtr;

	InstancePtr = &TimerInst;
	if (InstancePtr->XTickTimer_SetPriority)
		InstancePtr->XTickTimer_SetPriority(InstancePtr, Priority);
}
