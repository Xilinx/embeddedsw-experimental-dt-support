/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file xuartlite_intr_tapp_example.c
*
* This file contains a design example using the UartLite driver and
* hardware device using the interrupt mode for transmission of data.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b sv   06/08/06 Created for supporting Test App Interrupt examples
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs and minor changes
*		      for coding guidelnes.
* 2.01a ssb  01/11/01 Updated the example to be used with the SCUGIC in
*		      Zynq.
* 3.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xuartlite.h"
#include "xil_printf.h"
#include "xinterrupt_wrap.h"
#ifdef SDT
#include "xuartlite_example.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#ifndef SDT
#define UARTLITE_DEVICE_ID	  XPAR_UARTLITE_0_DEVICE_ID
#endif
#endif /* TESTAPP_GEN */

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the UartLite device.
 */
#define TEST_BUFFER_SIZE		100


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#ifndef SDT
int UartLiteIntrExample(XUartLite *UartLiteInstancePtr,
			u16 UartLiteDeviceId);
#else
int UartLiteIntrExample(XUartLite *UartLiteInstancePtr,
                        UINTPTR BaseAddress);
#endif

static void UartLiteSendHandler(void *CallBackRef, unsigned int EventData);

static void UartLiteRecvHandler(void *CallBackRef, unsigned int EventData);

/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs.
 */
#ifndef TESTAPP_GEN
static XUartLite UartLiteInst;  /* The instance of the UartLite Device */
#endif

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */

/*
 * The following buffer is used in this example to send data  with the UartLite.
 */
u8 SendBuffer[TEST_BUFFER_SIZE];

/*
 * The following counter is used to determine when the entire buffer has
 * been sent.
 */
static volatile int TotalSentCount;

/******************************************************************************/
/**
*
* Main function to call the UartLite interrupt example.
*
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Run the UartLite Interrupt example , specify the Device ID that is
	 * generated in xparameters.h.
	 */
#ifndef SDT
	Status = UartLiteIntrExample(&UartLiteInst,
				 UARTLITE_DEVICE_ID);
#else
	Status = UartLiteIntrExample(&UartLiteInst, XUARTLITE_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Uartlite interrupt tapp Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Uartlite interrupt tapp Example\r\n");
	return XST_SUCCESS;
}
#endif

/****************************************************************************/
/**
*
* This function does a minimal test on the UartLite device and driver as a
* design example. The purpose of this function is to illustrate how to use
* the XUartLite component.
*
* This function sends data through the UartLite.
*
* This function uses the interrupt driver mode of the UartLite.  The calls to
* the  UartLite driver in the interrupt handlers, should only use the
* non-blocking calls.
*
* @param	IntcInstancePtr is a pointer to the instance of INTC driver.
* @param	UartLiteInstPtr is a pointer to the instance of UartLite driver.
* @param	UartLiteDeviceId is the Device ID of the UartLite Device and
*		is the XPAR_<UARTLITE_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	UartLiteIntrId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<UARTLITE_instance>_VEC_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
* This function contains an infinite loop such that if interrupts are not
* working it may never return.
*
****************************************************************************/
#ifndef SDT
int UartLiteIntrExample(XUartLite *UartLiteInstPtr,
			u16 UartLiteDeviceId)
#else
int UartLiteIntrExample(XUartLite *UartLiteInstPtr,
			UINTPTR BaseAddress)
#endif
{
	int Status;
	u32 Index;
        XUartLite_Config *CfgPtr;

	/*
	 * Initialize the UartLite driver so that it's ready to use.
	 */
#ifndef SDT
	Status = XUartLite_Initialize(UartLiteInstPtr, UartLiteDeviceId);
        CfgPtr = XUartLite_LookupConfig(UartLiteDeviceId);
#else
        CfgPtr = XUartLite_LookupConfig(BaseAddress);
        Status = XUartLite_CfgInitialize(UartLiteInstPtr, CfgPtr,
                                CfgPtr->RegBaseAddr);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XUartLite_SelfTest(UartLiteInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the UartLite to the interrupt subsystem such that interrupts
	 * can occur. This function is application specific.
	 */
	Status = XSetupInterruptSystem(UartLiteInstPtr, &XUartLite_InterruptHandler,
					CfgPtr->IntrId, CfgPtr->IntrParent,
					XINTERRUPT_DEFAULT_PRIORITY);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the UartLite that will be called from the
	 * interrupt context when data has been sent and received,
	 * specify a pointer to the UartLite driver instance as the callback
	 * reference so the handlers are able to access the instance data.
	 */
	XUartLite_SetSendHandler(UartLiteInstPtr, UartLiteSendHandler,
							 UartLiteInstPtr);
	XUartLite_SetRecvHandler(UartLiteInstPtr, UartLiteRecvHandler,
							 UartLiteInstPtr);

	/*
	 * Enable the interrupt of the UartLite so that the interrupts
	 * will occur.
	 */
	XUartLite_EnableInterrupt(UartLiteInstPtr);

	/*
	 * Initialize the send buffer bytes with a pattern to send.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SendBuffer[Index] = Index;
	}

	/*
	 * Send the buffer using the UartLite.
	 */
	XUartLite_Send(UartLiteInstPtr, SendBuffer, TEST_BUFFER_SIZE);

	/*
	 * Wait for the entire buffer to be transmitted,  the function may get
	 * locked up in this loop if the interrupts are not working correctly.
	 */
	while ((TotalSentCount != TEST_BUFFER_SIZE)) {
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the handler which performs processing to send data to the
* UartLite. It is called from an interrupt context such that the amount of
* processing performed should be minimized. It is called when the transmit
* FIFO of the UartLite is empty and more data can be sent through the UartLite.
*
* This handler provides an example of how to handle data for the UartLite, but
* is application specific.
*
* @param	CallBackRef contains a callback reference from the driver.
*		In this case it is the instance pointer for the UartLite driver.
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void UartLiteSendHandler(void *CallBackRef, unsigned int EventData)
{
	TotalSentCount = EventData;
}

/****************************************************************************/
/**
*
* This function is the handler which performs processing to receive data from
* the UartLite. It is called from an interrupt context such that the amount of
* processing performed should be minimized. It is called when any data is
* present in the receive FIFO of the UartLite such that the data can be
* retrieved from the UartLite. The amount of data present in the FIFO is not
* known when this function is called.
*
* This handler provides an example of how to handle data for the UartLite, but
* is application specific.
*
* @param	CallBackRef contains a callback reference from the driver,
*		in this case it is the instance pointer for the UartLite driver.
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void UartLiteRecvHandler(void *CallBackRef, unsigned int EventData)
{

}


