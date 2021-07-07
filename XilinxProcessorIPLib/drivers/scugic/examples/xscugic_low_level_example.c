/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
*
* @file xscugic_low_level_example.c
*
* This file contains a design example using the low level driver, interface
* of the Interrupt Controller driver.
*
* This example shows the use of the Interrupt Controller with the ARM
* processor.
*
* @note
*
* none
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a drg  01/30/10 First release
* 3.10  mus  09/19/18 Update prototype of LowInterruptHandler to fix the GCC
*                     warning
* 4.0   mus  01/28/19  Updated to support Cortexa72 GIC (GIC500).
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic_hw.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "xscugic.h"

#ifdef SDT
#include "xscugic_example.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define INTC_DEVICE_ID          XPAR_SCUGIC_0_DEVICE_ID
#define CPU_BASEADDR		XPAR_SCUGIC_0_CPU_BASEADDR
#define DIST_BASEADDR		XPAR_SCUGIC_0_DIST_BASEADDR
#else
#define DIST_BASEADDR		XSCUGIC_BASEADDRESS
#endif

#define TARGETED_SGI_ID	0x3U

#if defined (versal) && !defined(ARMR5)
#define GIC_DEVICE_INT_MASK        0x13000001 /* Bit [27:24] SGI Interrupt ID
                                                 Bit [15:0] Targeted CPUs */
#else
#define GIC_DEVICE_INT_MASK        0x02010003 /* Bit [25:24] Target list filter
                                                 Bit [23:16] 16 = Target CPU iface 0
                                                 Bit [3:0] identifies the SFI */
#endif
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static int ScuGicLowLevelExample();

void SetupInterruptSystem();

void LowInterruptHandler(u32 CallbackRef);


/************************** Variable Definitions *****************************/

/*
 * Create a shared variable to be used by the main thread of processing and
 * the interrupt processing
 */
volatile static u32 InterruptProcessed = FALSE;

/*****************************************************************************/
/**
*
* This is the main function for the Interrupt Controller Low Level example.
*
* @param	None.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;


	/*
	 * Run the low level example of Interrupt Controller, specify the Base
	 * Address generated in xparameters.h
	 */
	xil_printf("Low Level GIC Example Test\r\n");
	Status = ScuGicLowLevelExample();

	if (Status != XST_SUCCESS) {
		xil_printf("Low Level GIC Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Low Level GIC Example Test\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is an example of how to use the interrupt controller driver
* (XScuGic) and the hardware device.  This function is designed to
* work without any hardware devices to cause interrupts.  It may not return
* if the interrupt controller is not properly connected to the processor in
* either software or hardware.
*
* This function relies on the fact that the interrupt controller hardware
* has come out of the reset state such that it will allow interrupts to be
* simulated by the software.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE
*
* @note		None.
*
******************************************************************************/
static int ScuGicLowLevelExample()
{

#ifndef SDT
	XScuGic_DeviceInitialize(INTC_DEVICE_ID);
#else
	XScuGic_DeviceInitialize(DIST_BASEADDR);
#endif
	
	/*
	 * This step is processor specific, connect the handler for the
	 * interrupt controller to the interrupt source for the processor
	 */
	SetupInterruptSystem();

	/*
	 * Enable targeted SGI
	 */
	XScuGic_EnableIntr(DIST_BASEADDR, TARGETED_SGI_ID);
	/*
	 * Cause (simulate) an interrupt so the handler will be called.
	 * This is done by changing the interrupt source to be software driven,
	 * then set a bit which simulates an interrupt.
	 */
#if defined (versal) && !defined(ARMR5)
	#if EL3
	XScuGic_WriteICC_SGI0R_EL1(GIC_DEVICE_INT_MASK);
    #else
	XScuGic_WriteICC_SGI1R_EL1(GIC_DEVICE_INT_MASK);
	#endif
#else
	XScuGic_WriteReg(DistBaseAddress, XSCUGIC_SFI_TRIG_OFFSET, GIC_DEVICE_INT_MASK);
#endif

	/*
	 * Wait for the interrupt to be processed, if the interrupt does not
	 * occur this loop will wait forever
	 */
	while (1)
	{
		/*
		 * If the interrupt occurred which is indicated by the global
		 * variable which is set in the device driver handler, then
		 * stop waiting
		 */
		if (InterruptProcessed != 0) {
			break;
		}
	}

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function connects the interrupt handler of the interrupt controller to
* the processor.  This function is separate to allow it to be customized for
* each application.  Each processor or RTOS may require unique processing to
* connect the interrupt handler.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void SetupInterruptSystem(void)
{
	XScuGic_Config *CfgPtr;
	#ifndef SDT
	XScuGic_LookupConfigBaseAddr(DIST_BASEADDR);
	#else
	CfgPtr = XScuGic_LookupConfig(DIST_BASEADDR);
	#endif
	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler) LowInterruptHandler,
			(void *)CfgPtr->CpuBaseAddress);

	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();
}

/*****************************************************************************/
/**
*
* This function is designed to look like an interrupt handler in a device
* driver. This is typically a 2nd level handler that is called from the
* interrupt controller interrupt handler.  This handler would typically
* perform device specific processing such as reading and writing the registers
* of the device to clear the interrupt condition and pass any data to an
* application using the device driver.
*
* @param    CallbackRef is passed back to the device driver's interrupt handler
*           by the XScuGic driver.  It was given to the XScuGic driver in the
*           XScuGic_Connect() function call.  It is typically a pointer to the
*           device driver instance variable if using the Xilinx Level 1 device
*           drivers.  In this example, we are passing it as scugic cpu
*           interface base address to access ack and EOI registers.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void LowInterruptHandler(u32 CallbackRef)
{
	u32 BaseAddress;
	u32 IntID;


	BaseAddress = CallbackRef;

#if defined (versal) && !defined(ARMR5)
	    IntID = XScuGic_get_IntID();
#else
	/*
	 * Read the int_ack register to identify the interrupt and
	 * make sure it is valid.
	 */
	IntID = XScuGic_ReadReg(BaseAddress, XSCUGIC_INT_ACK_OFFSET) &
			    XSCUGIC_ACK_INTID_MASK;
#endif
	if(XSCUGIC_MAX_NUM_INTR_INPUTS < IntID){
		return;
	}

	/*
	 * If the interrupt is shared, do some locking here if there are
	 * multiple processors.
	 */

	/*
	 * Execute the ISR. For this example set the global to 1.
	 * The software trigger is cleared by the ACK.
	 */
	InterruptProcessed = 1;

#if defined (versal) && !defined(ARMR5)
	   XScuGic_ack_Int(IntID);

#else
	/*
	 * Write to the EOI register, we are all done here.
	 * Let this function return, the boot code will restore the stack.
	 */
	XScuGic_WriteReg(BaseAddress, XSCUGIC_EOI_OFFSET, IntID);
#endif
}

