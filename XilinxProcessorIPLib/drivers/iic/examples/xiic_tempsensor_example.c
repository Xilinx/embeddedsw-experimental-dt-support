/******************************************************************************
* Copyright (C) 2006 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiic_tempsensor_example.c
*
* This file contains an interrupt based design example which uses the Xilinx
* IIC device and driver to exercise the temperature sensor on the ML300 board.
* This example only performs read operations (receive) from the IIC temperature
* sensor of the platform.
*
* The XIic_MasterRecv() API is used to receive the data.
*
* This example assumes that there is an interrupt controller in the hardware
* system and the IIC device is connected to the interrupt controller.
*
* @note
*
* 7-bit addressing is used to access the tempsensor.
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a jhl  09/10/03 Created
* 1.00a sv   05/09/05 Minor changes to comply to Doxygen and coding guidelines
* 3.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

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
 * temperature sensor device on the IIC bus. Note that since
 * the address is only 7 bits, this  constant is the address divided by 2.
 */
#define TEMP_SENSOR_ADDRESS	0x18 /* The actual address is 0x30 */


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ****************************/
#ifndef SDT
int TempSensorExample(u16 IicDeviceId, u8 TempSensorAddress,
						  u8 *TemperaturePtr);
#else
int TempSensorExample(UINTPTR BaseAddress, u8 TempSensorAddress,
                                                  u8 *TemperaturePtr);
#endif
static void RecvHandler(void *CallbackRef, int ByteCount);

static void StatusHandler(void *CallbackRef, int Status);


/************************** Variable Definitions **************************/

XIic Iic;		  /* The instance of the IIC device */

/*
 * The following structure contains fields that are used with the callbacks
 * (handlers) of the IIC driver. The driver asynchronously calls handlers
 * when abnormal events occur or when data has been sent or received. This
 * structure must be volatile to work when the code is optimized.
 */
volatile struct {
	int  EventStatus;
	int  RemainingRecvBytes;
	int EventStatusUpdated;
	int RecvBytesUpdated;
} HandlerInfo;

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the IIC driver to
* read the temperature.
*
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None
*
*******************************************************************************/
int main(void)
{
	int Status;
	u8 TemperaturePtr;

	/*
	 * Call the TempSensorExample.
	 */
#ifndef SDT
	Status =  TempSensorExample(IIC_DEVICE_ID, TEMP_SENSOR_ADDRESS,
							&TemperaturePtr);
#else
	 Status =  TempSensorExample(XIIC_BASEADDRESS, TEMP_SENSOR_ADDRESS,
                                                        &TemperaturePtr);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("IIC tempsensor Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC tempsensor Example\r\n");
	return XST_SUCCESS;

}

/*****************************************************************************/
/**
* The function reads the temperature of the IIC temperature sensor on the
* IIC bus. It initializes the IIC device driver and sets it up to communicate
* with the temperature sensor. This function does contain a loop that polls
* for completion of the IIC processing such that it may not return if
* interrupts or the hardware are not working.
*
* @param	IicDeviceId is the XPAR_<IIC_instance>_DEVICE_ID value from
*		xparameters.h for the IIC Device
* @param	TempSensorAddress is the address of the Temperature Sensor device
*		on the IIC bus
* @param	TemperaturePtr is the data byte read from the temperature sensor
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate
*		a Failure.
*
* @note		None.
*
*******************************************************************************/
#ifndef SDT
int TempSensorExample(u16 IicDeviceId, u8 TempSensorAddress, u8 *TemperaturePtr)
#else
int TempSensorExample(UINTPTR BaseAddress, u8 TempSensorAddress, u8 *TemperaturePtr)
#endif
{
	int Status;
	static int Initialized = FALSE;
	XIic_Config *ConfigPtr;	/* Pointer to configuration data */

	if (!Initialized) {
		Initialized = TRUE;

		/*
		 * Initialize the IIC driver so that it is ready to use.
		 */
#ifndef SDT
		ConfigPtr = XIic_LookupConfig(IicDeviceId);
#else
		ConfigPtr = XIic_LookupConfig(BaseAddress);
#endif
		if (ConfigPtr == NULL) {
			return XST_FAILURE;
		}

		Status = XIic_CfgInitialize(&Iic, ConfigPtr,
						ConfigPtr->BaseAddress);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}


		/*
		 * Setup handler to process the asynchronous events which occur,
		 * the driver is only interrupt driven such that this must be
		 * done prior to starting the device.
		 */
		XIic_SetRecvHandler(&Iic, (void *)&HandlerInfo, RecvHandler);
		XIic_SetStatusHandler(&Iic, (void *)&HandlerInfo,
						StatusHandler);

		/*
		 * Connect the ISR to the interrupt and enable interrupts.
		 */
		Status = XSetupInterruptSystem(&Iic, &XIic_InterruptHandler,
						ConfigPtr->IntrId, ConfigPtr->IntrParent,
						XINTERRUPT_DEFAULT_PRIORITY);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Start the IIC driver such that it is ready to send and
		 * receive messages on the IIC interface, set the address
		 * to send to which is the temperature sensor address
		 */
		XIic_Start(&Iic);
		XIic_SetAddress(&Iic, XII_ADDR_TO_SEND_TYPE, TempSensorAddress);
	}

	/*
	 * Clear updated flags such that they can be polled to indicate
	 * when the handler information has changed asynchronously and
	 * initialize the status which will be returned to a default value
	 */
	HandlerInfo.EventStatusUpdated = FALSE;
	HandlerInfo.RecvBytesUpdated = FALSE;
	Status = XST_FAILURE;

	/*
	 * Attempt to receive a byte of data from the temperature sensor
	 * on the IIC interface, ignore the return value since this example is
	 * a single master system such that the IIC bus should not ever be busy
	 */
	(void)XIic_MasterRecv(&Iic, TemperaturePtr, 1);

	/*
	 * The message is being received from the temperature sensor,
	 * wait for it to complete by polling the information that is
	 * updated asynchronously by interrupt processing
	 */
	while(1) {
		if(HandlerInfo.RecvBytesUpdated == TRUE) {
			/*
			 * The device information has been updated for receive
			 * processing,if all bytes received (1), indicate
			 * success
			 */
			if (HandlerInfo.RemainingRecvBytes == 0) {
				Status = XST_SUCCESS;
			}
			break;
		}

		/*
		 * Any event status which occurs indicates there was an error,
		 * so return unsuccessful, for this example there should be no
		 * status events since there is a single master on the bus
		 */
		if (HandlerInfo.EventStatusUpdated == TRUE) {
			break;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* This receive handler is called asynchronously from an interrupt context and
* and indicates that data in the specified buffer was received. The byte count
* should equal the byte count of the buffer if all the buffer was filled.
*
* @param	CallBackRef is a pointer to the IIC device driver instance which
*		the handler is being called for.
* @param	ByteCount indicates the number of bytes remaining to be received of
*		the requested byte count. A value of zero indicates all requested
*		bytes were received.
*
* @return	None.
*
* @notes	None.
*
****************************************************************************/
static void RecvHandler(void *CallbackRef, int ByteCount)
{
	HandlerInfo.RemainingRecvBytes = ByteCount;
	HandlerInfo.RecvBytesUpdated = TRUE;
}

/*****************************************************************************/
/**
* This status handler is called asynchronously from an interrupt context and
* indicates that the conditions of the IIC bus changed. This  handler should
* not be called for the application unless an error occurs.
*
* @param	CallBackRef is a pointer to the IIC device driver instance which the
*		handler is being called for.
* @param	Status contains the status of the IIC bus which changed.
*
* @return	None.
*
* @notes	None.
*
****************************************************************************/
static void StatusHandler(void *CallbackRef, int Status)
{
	HandlerInfo.EventStatus |= Status;
	HandlerInfo.EventStatusUpdated = TRUE;
}
