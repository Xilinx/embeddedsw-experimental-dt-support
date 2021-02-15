/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xiicps_slave_monitor_example.c
 *
 * A design example of using the device as master to check slave's
 * availability.
 *
 * @note
 * Please set the slave address to 0x3FB, which tests the device's ability
 *	to handle 10-bit address.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00a jz  01/30/10 First release
 *
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xiicps.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xinterrupt_wrap.h"

/************************** Constant Definitions ******************************/

#define IIC_SCLK_RATE		100000
#define SLV_MON_LOOP_COUNT 0x000FFFFF	/**< Slave Monitor Loop Count*/

/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

int IicPsSlaveMonitorExample();

static void Handler(void *CallBackRef, u32 Event);
#ifndef SDT
static int IicPsSlaveMonitor(u16 Address, u16 DeviceId);
static int IicPsConfig(u16 DeviceId);
#else
static int IicPsSlaveMonitor(u16 Address, UINTPTR BaseAddress);
static int IicPsConfig(UINTPTR BaseAddress);
#endif
static int IicPsFindDevice(u16 Addr);

/************************** Variable Definitions ******************************/

XIicPs	Iic;			/* Instance of the IIC Device */

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
volatile u8 TransmitComplete;	/**< Flag to check completion of Transmission */
volatile u8 ReceiveComplete;	/**< Flag to check completion of Reception */
volatile u32 TotalErrorCount;	/**< Total Error Count Flag */
volatile u32 SlaveResponse;		/**< Slave Response Flag */

/**Searching for the required Slave Address and user can also add
 * their own slave Address in the below array list**/
u16 SlvAddr[] = {0x54,0x55,0x74,0};
XIicPs IicInstance;		/* The instance of the IIC device. */
/******************************************************************************/
/**
*
* Main function to call the Slave Monitor example.
*
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
*******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("IIC Slave Monitor Example Test \r\n");

	/*
	 * Run the Iic Slave Monitor example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = IicPsSlaveMonitorExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC Slave Monitor Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran IIC Slave Monitor Example Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function perform the initial configuration for the IICPS Device.
*
* @param	DeviceId instance and Interrupt ID mapped to the device.
*
* @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
#ifndef SDT
static int IicPsConfig(u16 DeviceId)
#else
static int IicPsConfig(UINTPTR BaseAddress)
#endif
{
	int Status;
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#ifndef SDT
	ConfigPtr = XIicPs_LookupConfig(DeviceId);
#else
	ConfigPtr = XIicPs_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&IicInstance, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the Interrupt System.
	 */
	Status = XSetupInterruptSystem(&IicInstance, XIicPs_MasterInterruptHandler,
					ConfigPtr->IntrId,
					ConfigPtr->IntrParent,
					XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handlers for the IIC that will be called from the
	 * interrupt context when data has been sent and received, specify a
	 * pointer to the IIC driver instance as the callback reference so
	 * the handlers are able to access the instance data.
	 */
	XIicPs_SetStatusHandler(&IicInstance, (void *) &IicInstance, Handler);

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&IicInstance, IIC_SCLK_RATE);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function checks the availability of a slave using slave monitor mode.
*
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note 	None.
*
*******************************************************************************/
int IicPsSlaveMonitorExample(void)
{
	int Status;
	int Index;

	for(Index = 0;SlvAddr[Index] != 0;Index++) {
		Status = IicPsFindDevice(SlvAddr[Index]);
		if (Status == XST_SUCCESS) {
			return XST_SUCCESS;
		}
	}
	return XST_FAILURE;
}
/*****************************************************************************/
/**
*
* This function checks the availability of a slave using slave monitor mode.
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*		XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note 	None.
*
*******************************************************************************/
#ifndef SDT
static int IicPsSlaveMonitor(u16 Address, u16 DeviceId)
#else
static int IicPsSlaveMonitor(u16 Address, UINTPTR BaseAddress)
#endif
{
	u32 Index;
	int Status;
	XIicPs *IicPtr;

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
#ifndef SDT
	Status = IicPsConfig(DeviceId);
#else
	Status = IicPsConfig(BaseAddress);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	IicPtr = &IicInstance;
	XIicPs_DisableAllInterrupts(IicPtr->Config.BaseAddress);
	XIicPs_EnableSlaveMonitor(&IicInstance, Address);

	TotalErrorCount = 0;
	SlaveResponse = FALSE;

	Index = 0;

	/*
	 * Wait for the Slave Monitor Interrupt, the interrupt processing
	 * works in the background, this function may get locked up in this
	 * loop if the interrupts are not working correctly or the slave
	 * never responds.
	 */
	while ((!SlaveResponse) && (Index < SLV_MON_LOOP_COUNT)) {
		Index++;

		/*
		 * Ignore any errors. The hardware generates NACK interrupts
		 * if the slave is not present.
		 */
		if (0 != TotalErrorCount) {
			xil_printf("Test error unexpected NACK\n");
			return XST_FAILURE;
		}
	}

	if (Index >= SLV_MON_LOOP_COUNT) {
		return XST_FAILURE;

	}

	XIicPs_DisableSlaveMonitor(&IicInstance);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function checks whether the slave in alive or not.
*
* @param	Addr : Address of the slave device
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note 	None.
*
*******************************************************************************/
static int IicPsFindDevice(u16 Addr)
{
	int Status;

	Status = IicPsSlaveMonitor(Addr,0);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
	Status = IicPsSlaveMonitor(Addr,1);
	if (Status == XST_SUCCESS) {
		return XST_SUCCESS;
	}
	return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function is the handler which performs processing to handle data events
* from the IIC.  It is called from an interrupt context such that the amount
* of processing performed should be minimized.
*
* This handler provides an example of how to handle data for the IIC and
* is application specific.
*
* @param	CallBackRef contains a callback reference from the driver, in
*		this case it is the instance pointer for the IIC driver.
* @param	Event contains the specific kind of event that has occurred.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void Handler(void *CallBackRef, u32 Event)
{
	/*
	 * All of the data transfer has been finished.
	 */

	if (0 != (Event & XIICPS_EVENT_COMPLETE_SEND)) {
		TransmitComplete = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_COMPLETE_RECV)){
		ReceiveComplete = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_SLAVE_RDY)) {
		SlaveResponse = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_ERROR)){
		TotalErrorCount++;
	}
}
