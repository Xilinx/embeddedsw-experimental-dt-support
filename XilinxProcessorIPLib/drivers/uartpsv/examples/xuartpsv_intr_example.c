/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file	xuartpsv_intr_example.c
*
* This file contains a design example using the XUartPsv driver in interrupt
* mode. It sends data and expects to receive the same data through the device
* using the local loopback mode.
*
*
* @note
* The example contains an infinite loop such that if interrupts are not
* working it may hang.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who   Date     Changes
* ----- ----- -------- ----------------------------------------------
* 1.0   ms    11/30/17 First Release
* 1.1   sd    07/11/19 Remove the hardcoded interrupt id.
* 1.2   rna   01/20/20 Add selftest, while loop for waiting
* </pre>
****************************************************************************/

/***************************** Include Files *******************************/

#include "xparameters.h"
#include "xplatform_info.h"
#include "xuartpsv.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xinterrupt_wrap.h"
#ifdef SDT
#include "xuartpsv_example.h"
#endif

/************************** Constant Definitions **************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define UARTPSV_DEVICE_ID               XPAR_XUARTPSV_0_DEVICE_ID
#endif

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the UART,
 */
#define TEST_BUFFER_SIZE	60


/**************************** Type Definitions ******************************/


/************************** Function Prototypes *****************************/
#ifndef SDT
int UartPsvIntrExample(XUartPsv *UartInstPtr, u16 DeviceId);
#else
int UartPsvIntrExample(XUartPsv *UartInstPtr, UINTPTR BaseAddress);
#endif

void Handler(void *CallBackRef, u32 Event, unsigned int EventData);


/************************** Variable Definitions ***************************/

XUartPsv UartPsv	;		/* Instance of the UART Device */

/*
 * The following buffers are used in this example to send and receive data
 * with the UART.
 */
static u8 SendBuffer[TEST_BUFFER_SIZE];	/* Buffer for Transmitting Data */
static u8 RecvBuffer[TEST_BUFFER_SIZE];	/* Buffer for Receiving Data */

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
volatile int TotalReceivedCount;
volatile int TotalSentCount;
int TotalErrorCount;

/**************************************************************************/
/**
*
* Main function to call the Uart interrupt example.
*
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
**************************************************************************/
#ifndef TESTAPP_GEN

int main(void)
{
	int Status;

	/* Run the UartPsv Interrupt example, specify the the Device ID */
#ifndef SDT
	Status = UartPsvIntrExample(&UartPsv, UARTPSV_DEVICE_ID);
#else
	Status = UartPsvIntrExample(&UartPsv, XUARTPSV_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("UartPsv Interrupt Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran UartPsv Interrupt Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/**************************************************************************/
/**
*
* This function does a minimal test on the UartPsv device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XUartPsv driver.
*
* This function sends data and expects to receive the same data through the
* device using the local loopback mode.
*
* This function uses interrupt mode of the device.
*
* @param	IntcInstPtr is a pointer to the instance of the Scu Gic driver.
* @param	UartInstPtr is a pointer to the instance of the UART driver
*		which is going to be connected to the interrupt controller.
* @param	DeviceId is the device Id of the UART device and is typically
*		XPAR_<UartPsv_instance>_DEVICE_ID value from xparameters.h.
* @param	UartIntrId is the interrupt Id and is typically
*		XPAR_<UartPsv_instance>_INTR value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
* This function contains an infinite loop such that if interrupts are not
* working it may never return.
*
**************************************************************************/
#ifndef SDT
int UartPsvIntrExample(XUartPsv *UartInstPtr, u16 DeviceId)
#else
int UartPsvIntrExample(XUartPsv *UartInstPtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	XUartPsv_Config *Config;
	int Index;
	u32 IntrMask;
	int BadByteCount = 0;

	/*
	 * Initialize the UART driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
#ifndef SDT
	Config = XUartPsv_LookupConfig(DeviceId);
#else
	Config = XUartPsv_LookupConfig(BaseAddress);
#endif
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XUartPsv_CfgInitialize(UartInstPtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Check hardware build */
	Status = XUartPsv_SelfTest(UartInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the UART to the interrupt subsystem such that interrupts
	 * can occur. This function is application specific.
	 */
	Status = XSetupInterruptSystem(UartInstPtr, &XUartPsv_InterruptHandler,
					Config->IntrId, Config->IntrParent,
					XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the UART that will be called from the
	 * interrupt context when data has been sent and received, specify
	 * a pointer to the UART driver instance as the callback reference
	 * so the handlers are able to access the instance data
	 */
	XUartPsv_SetHandler(UartInstPtr, (XUartPsv_Handler)Handler, UartInstPtr);


	/* Configure the Rx fifo level less than Tx Fifo level, since we are
	 * using in Loopback mode. If not, the Tx interrupt comes first before Rx
	 * interrupt and results in an over run at the Rx end.
	 */

	XUartPsv_SetRxFifoThreshold(UartInstPtr, XUARTPSV_UARTIFLS_RXIFLSEL_1_4);

	XUartPsv_SetTxFifoThreshold(UartInstPtr, XUARTPSV_UARTIFLS_TXIFLSEL_1_2);
	/*
	 * Enable the interrupt of the UART so interrupts will occur, setup
	 * a local loop back so data that is sent will be received.
	 */
	IntrMask = (XUARTPSV_UARTIMSC_RXIM | XUARTPSV_UARTIMSC_TXIM |
			XUARTPSV_UARTIMSC_RTIM | XUARTPSV_UARTIMSC_FEIM |
			XUARTPSV_UARTIMSC_PEIM | XUARTPSV_UARTIMSC_BEIM |
			XUARTPSV_UARTIMSC_OEIM);


	XUartPsv_SetInterruptMask(UartInstPtr, IntrMask);

	XUartPsv_SetOperMode(UartInstPtr, XUARTPSV_OPER_MODE_LOCAL_LOOP);

	/*
	 * Initialize the send buffer bytes with a pattern and the
	 * the receive buffer bytes to zero to allow the receive data to be
	 * verified
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {

		SendBuffer[Index] = (Index % 26) + 'A';

		RecvBuffer[Index] = 0;
	}

	/*
	 * Start receiving data before sending it since there is a loopback,
	 * ignoring the number of bytes received as the return value since we
	 * know it will be zero
	 */
	XUartPsv_Recv(UartInstPtr, RecvBuffer, TEST_BUFFER_SIZE);

	/*
	 * Send the buffer using the UART and ignore the number of bytes sent
	 * as the return value since we are using it in interrupt mode.
	 */
	XUartPsv_Send(UartInstPtr, SendBuffer, TEST_BUFFER_SIZE);

	/* Wait until all the data is received or any error occurs.
	 * This can hang if interrupts are not working properly in the
	 * background
	 */
	while ((TotalErrorCount == 0) && ((TotalSentCount != TEST_BUFFER_SIZE)
			|| (TotalReceivedCount != TEST_BUFFER_SIZE)));


	/* Verify the entire receive buffer was successfully received */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		if (RecvBuffer[Index] != SendBuffer[Index]) {
			BadByteCount++;
		}
	}

	/* Set the UART in Normal Mode */
	XUartPsv_SetOperMode(UartInstPtr, XUARTPSV_OPER_MODE_NORMAL);

	/* If any bytes were not correct, return an error */
	if (BadByteCount != 0) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/**************************************************************************/
/**
*
* This function is the handler which performs processing to handle data events
* from the device.  It is called from an interrupt context. so the amount of
* processing should be minimal.
*
* This handler provides an example of how to handle data for the device and
* is application specific.
*
* @param	CallBackRef contains a callback reference from the driver,
*		in this case it is the instance pointer for the XUARTPSV driver.
* @param	Event contains the specific kind of event that has occurred.
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
*
* @return	None.
*
* @note		None.
*
***************************************************************************/
void Handler(void *CallBackRef, u32 Event, unsigned int EventData)
{
	/* All of the data has been sent */
	if (Event == XUARTPSV_EVENT_SENT_DATA) {
		TotalSentCount = EventData;
	}

	/* All of the data has been received */
	if (Event == XUARTPSV_EVENT_RECV_DATA) {
		TotalReceivedCount = EventData;
	}

	/*
	 * Data was received, but not the expected number of bytes, a
	 * timeout just indicates the data stopped for 8 character times
	 */
	if (Event == XUARTPSV_EVENT_RECV_TOUT) {
		TotalReceivedCount = EventData;
	}

	/*
	 * Data was received with an error, keep the data but determine
	 * what kind of errors occurred
	 */
	if (Event == XUARTPSV_EVENT_RECV_ERROR) {
		TotalReceivedCount = EventData;
		TotalErrorCount++;
	}

	/*
	 * Data was received with an parity or frame or break error, keep the data
	 * but determine what kind of errors occurred. Specific to Zynq Ultrascale+
	 * MP.
	 */
	if (Event == XUARTPSV_EVENT_PARE_FRAME_BRKE) {
		TotalReceivedCount = EventData;
		TotalErrorCount++;
	}

	/*
	 * Data was received with an overrun error, keep the data but determine
	 * what kind of errors occurred. Specific to Zynq Ultrascale+ MP.
	 */
	if (Event == XUARTPSV_EVENT_RECV_ORERR) {
		TotalReceivedCount = EventData;
		TotalErrorCount++;
	}
}

