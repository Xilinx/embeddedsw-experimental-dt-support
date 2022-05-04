/******************************************************************************
* Copyright (c) 2021-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
*@file xiltimer.c
*
* This file contains the sleep API's
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
*  1.0  adk	 24/11/21 Initial release.
*  	adk	 07/02/22 Integrate configuring of priority field into
*  			  XTimer_SetHandler() API instead of a new API.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "sleep.h"
#include "xiltimer.h"

/****************************  Constant Definitions  *************************/
XTimer TimerInst;
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

/****************************************************************************/
/**
*
* This API initializes the xiltimer library through constructor.
*
* @param            none
*
* @return           none
*
* @note             none
*
*****************************************************************************/
void __attribute__ ((constructor)) xtimerinit()
{
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
* @param        Priority - Priority for the interrupt
*
* @return	None
*
*****************************************************************************/
void XTimer_SetHandler(XTimer_TickHandler FuncPtr, void *CallBackRef,
		       u8 Priority)
{
	XTimer *InstancePtr;

	InstancePtr = &TimerInst;

	InstancePtr->Handler = FuncPtr;
	InstancePtr->CallBackRef = CallBackRef;
	if (InstancePtr->XTimer_TickIntrHandler) {
		InstancePtr->XTimer_TickIntrHandler(InstancePtr, Priority);
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

/****************************************************************************/
/**
*
* This API clears the interrupt status of the tick timer instance.
*
* @param            none
*
* @return           none
*
* @note             none
*
*****************************************************************************/
void XTimer_ClearTickInterrupt( void )
{
	XTimer *InstancePtr;

	InstancePtr = &TimerInst;
	if (InstancePtr->XTickTimer_ClearInterrupt)
		InstancePtr->XTickTimer_ClearInterrupt(InstancePtr);
}
