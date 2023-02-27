/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xtmrctr_selftest_example.c
*
* This file contains a example for  using the Timer Counter hardware and
* driver
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sv   04/25/05 Initial release for TestApp integration.
* 2.00a ktn  11/26/09 Minor changes as per coding guidelines.
* 4.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xtmrctr.h"
#include "xil_printf.h"

#ifdef SDT
#include "xtmrctr_example.h"
#else
#include "xparameters.h"
#endif

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define TMRCTR_DEVICE_ID  XPAR_TMRCTR_0_DEVICE_ID
#endif

/*
 * This example only uses the 1st of the 2 timer counters contained in a
 * single timer counter hardware device
 */
#define TIMER_COUNTER_0	 0

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions *******************/


/************************** Function Prototypes ****************************/
#ifndef SDT
int TmrCtrSelfTestExample(u16 DeviceId, u8 TmrCtrNumber);
#else
int TmrCtrSelfTestExample(UINTPTR BaseAddr, u8 TmrCtrNumber);
#endif

/************************** Variable Definitions **************************/

XTmrCtr TimerCounter; /* The instance of the timer counter */


/*****************************************************************************/
/**
* Main function to call the example. This function is not included if the
* example is generated from the TestAppGen test tool.
*
* @param	None
*
* @return   XST_SUCCESS to indicate success, else XST_FAILURE to indicate
*		   a Failure.
*
* @note	 None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;
#ifndef SDT
	Status = TmrCtrSelfTestExample(TMRCTR_DEVICE_ID, TIMER_COUNTER_0);
#else
	Status = TmrCtrSelfTestExample(XTMRCTR_BASEADDRESS, TIMER_COUNTER_0);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Tmrctr selftest Example\r\n");
	return XST_SUCCESS;
}
#endif


/*****************************************************************************/
/**
*
* This function does a minimal test on the TmrCtr device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XTmrCtr component.
*
*
* @param	DeviceId is the XPAR_<TMRCTR_instance>_DEVICE_ID value from
*		xparameters.h
* @param	TmrCtrNumber is the timer counter of the device to operate on.
*		Each device may contain multiple timer counters.
*		The timer number is a zero based number with a range of
*		0 - (XTC_DEVICE_TIMER_COUNT - 1).
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
****************************************************************************/
#ifndef SDT
int TmrCtrSelfTestExample(u16 DeviceId, u8 TmrCtrNumber)
#else
int TmrCtrSelfTestExample(UINTPTR BaseAddr, u8 TmrCtrNumber)
#endif
{
	int Status;
	XTmrCtr *TmrCtrInstancePtr = &TimerCounter;

	/*
	 * Initialize the TmrCtr driver so that it iss ready to use
	 */
	#ifndef SDT
	Status = XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);
	#else
	Status = XTmrCtr_Initialize(TmrCtrInstancePtr, BaseAddr);
	#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly, use the 1st timer in the device (0)
	 */
	Status = XTmrCtr_SelfTest(TmrCtrInstancePtr, TmrCtrNumber);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
