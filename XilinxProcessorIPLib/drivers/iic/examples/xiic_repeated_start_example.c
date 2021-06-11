/******************************************************************************
* Copyright (C) 2006 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiic_repeated_start_example.c
*
* This file consists of a interrupt mode design example to demonstrate the use
* of repeated start using the XIic driver.
*
* The XIic_MasterSend() API is used to transmit the data and XIic_MasterRecv()
* API is used to receive the data.
*
* The IIC devices that are present on the Xilinx boards donot support the
* repeated start option. These examples have been tested with an IIC
* device external to the boards.
*
* This code assumes that no Operating System is being used.
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
* 1.00a mta  02/20/06 Created.
* 2.00a sdm  09/22/09 Updated to use the HAL APIs, replaced call to
*		      XIic_Initialize API with XIic_LookupConfig and
*		      XIic_CfgInitialize. Updated the example with a
*	              fix for CR539763 where XIic_Start was being called
*	              instead of XIic_Stop. Added code for setting up the
*	              StatusHandler callback.
* 3.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xinterrupt_wrap.h"
#ifdef SDT
#include "xiic_example.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define IIC_DEVICE_ID		XPAR_IIC_0_DEVICE_ID
#endif

/*
 * The following constant defines the address of the IIC
 * device on the IIC bus. Note that since the address is only 7 bits, this
 * constant is the address divided by 2.
 */
#define SLAVE_ADDRESS	0x70	/* 0xE0 as an 8 bit number. */

#define SEND_COUNT	16
#define RECEIVE_COUNT   16

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int IicRepeatedStartExample();
static int WriteData(u16 ByteCount);
static int ReadData(u8 *BufferPtr, u16 ByteCount);
static void SendHandler(XIic *InstancePtr);
static void ReceiveHandler(XIic *InstancePtr);
static void StatusHandler(XIic *InstancePtr, int Event);

/************************** Variable Definitions *****************************/

XIic IicInstance;

u8 WriteBuffer[SEND_COUNT];	/* Write buffer for writing a page. */
u8 ReadBuffer[RECEIVE_COUNT];	/* Read buffer for reading a page. */

volatile u8 TransmitComplete;
volatile u8 ReceiveComplete;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* Main function to call the Repeated Start example.
*
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the Repeated Start example.
	 */
	Status = IicRepeatedStartExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC repeated start Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC repeated start Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes and reads the data to the IIC Slave.
*
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int IicRepeatedStartExample(void)
{
	u8 Index;
	int Status;
	XIic_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Initialize the data to write and the read buffer.
	 */
	for (Index = 0; Index < SEND_COUNT; Index++) {
		WriteBuffer[Index] = Index;
		ReadBuffer[Index] = 0;
	}

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#ifndef SDT
	ConfigPtr = XIic_LookupConfig(XPAR_IIC_0_DEVICE_ID);
#else
	ConfigPtr = XIic_LookupConfig(XIIC_BASEADDRESS);
#endif
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(&IicInstance, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the Interrupt System.
	 */
	Status = XSetupInterruptSystem(&IicInstance, &XIic_InterruptHandler,
					ConfigPtr->IntrId, ConfigPtr->IntrParent,
					XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the Transmit, Receive and Status handlers.
	 */
	XIic_SetSendHandler(&IicInstance, &IicInstance,
				(XIic_Handler) SendHandler);
	XIic_SetRecvHandler(&IicInstance, &IicInstance,
				(XIic_Handler) ReceiveHandler);
	XIic_SetStatusHandler(&IicInstance, &IicInstance,
				  (XIic_StatusHandler) StatusHandler);

	/*
	 * Set the Address of the Slave.
	 */
	Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE,
				 SLAVE_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Write to the IIC Slave.
	 */
	Status = WriteData(SEND_COUNT);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Read from the IIC Slave.
	 */
	Status = ReadData(ReadBuffer, RECEIVE_COUNT);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes a buffer of data to IIC Slave.
*
* @param	ByteCount contains the number of bytes in the buffer to be
*		written.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int WriteData(u16 ByteCount)
{
	int Status;
	int BusBusy;

	/*
	 * Set the defaults.
	 */
	TransmitComplete = 1;

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the Repeated Start option.
	 */
	IicInstance.Options = XII_REPEATED_START_OPTION;

	/*
	 * Send the data.
	 */
	Status = XIic_MasterSend(&IicInstance, WriteBuffer, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till data is transmitted.
	 */
	while (TransmitComplete) {

	}

	/*
	 * This is for verification that Bus is not released and still Busy.
	 */
	BusBusy = XIic_IsIicBusy(&IicInstance);

	TransmitComplete = 1;
	IicInstance.Options = 0x0;

	/*
	 * Send the Data.
	 */
	Status = XIic_MasterSend(&IicInstance, WriteBuffer, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till data is transmitted.
	 */
	while ((TransmitComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {

	}

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function reads a data from the IIC Slave into a specified buffer.
*
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes to be read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int ReadData(u8 *BufferPtr, u16 ByteCount)
{
	int Status;
	int BusBusy;

	/*
	 * Set the defaults.
	 */
	ReceiveComplete = 1;

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the Repeated Start option.
	 */
	IicInstance.Options = XII_REPEATED_START_OPTION;

	/*
	 * Receive the data.
	 */
	Status = XIic_MasterRecv(&IicInstance, BufferPtr, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till all the data is received.
	 */
	while (ReceiveComplete) {

	}

	/*
	 * This is for verification that Bus is not released and still Busy.
	 */
	BusBusy = XIic_IsIicBusy(&IicInstance);

	ReceiveComplete = 1;
	IicInstance.Options = 0x0;

	/*
	 * Receive the Data.
	 */
	Status = XIic_MasterRecv(&IicInstance, BufferPtr, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait till all the data is received.
	 */
	while ((ReceiveComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {

	}

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This Send handler is called asynchronously from an interrupt context and
* indicates that data in the specified buffer has been sent.
*
* @param	InstancePtr is a pointer to the IIC driver instance for which
* 		the handler is being called for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SendHandler(XIic *InstancePtr)
{
	TransmitComplete = 0;
}

/*****************************************************************************/
/**
* This Receive handler is called asynchronously from an interrupt context and
* indicates that data in the specified buffer has been Received.
*
* @param	InstancePtr is a pointer to the IIC driver instance for which
* 		the handler is being called for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void ReceiveHandler(XIic *InstancePtr)
{
	ReceiveComplete = 0;
}

/*****************************************************************************/
/**
* This Status handler is called asynchronously from an interrupt
* context and indicates the events that have occurred.
*
* @param	InstancePtr is a pointer to the IIC driver instance for which
*		the handler is being called for.
* @param	Event indicates the condition that has occurred.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StatusHandler(XIic *InstancePtr, int Event)
{

}
