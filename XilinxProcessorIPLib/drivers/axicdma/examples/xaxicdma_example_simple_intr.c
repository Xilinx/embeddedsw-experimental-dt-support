/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxicdma_example_simple_intr.c
 *
 * This file demonstrates how to use the xaxicdma driver on the Xilinx AXI
 * CDMA core (AXICDMA) to transfer packets in simple transfer mode through
 * interrupt.
 *
 * Modify the NUMBER_OF_TRANSFER constant to have different number of simple
 * transfers done in this test.
 *
 * This example assumes that the system has an interrupt controller.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options for the example, also
 * comment out the "#undef DEBUG" in xdebug.h. You need to rebuild your
 * software executable.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 *  . Updated the debug print on type casting to avoid warnings on u32. Cast
 *    u32 to (unsigned int) to use the %x format.
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   07/30/10 First release
 * 2.01a rkv  01/28/11 Changed function prototype of XAxiCdma_SimpleIntrExample
 * 		       to a function taking arguments interrupt instance,device
 * 		       instance,device id,device interrupt id
 *		       Added interrupt support for Cortex A9
 * 2.01a srt  03/05/12 Modified interrupt support for Zynq.
 * 		       Modified Flushing and Invalidation of Caches to fix CRs
 *		       648103, 648701.
 * 4.3   ms   01/22/17 Modified xil_printf statement in main function to
 *            ensure that "Successfully ran" and "Failed" strings are
 *            available in all examples. This is a fix for CR-965028.
 *       ms   04/05/17 Modified Comment lines in functions to
 *                     recognize it as documentation block for doxygen
 *                     generation of examples.
 * 4.4   rsp  02/22/18 Support data buffers above 4GB.Use UINTPTR for
 *                     typecasting buffer address(CR-995116).
 * 4.6   rsp  09/13/19 Add error prints for failing scenarios.
 *                     Fix cache maintenance ops for source and dest buffer.
 * 4.7   rsp  12/06/19 For aarch64 include xil_mmu.h. Fixes gcc warning.
 * 4.8	 sk   09/28/20 Fix the compilation error for xreg_cortexa9.h
 * 		       preprocessor on R5 processor.
 * 4.11  sa   09/29/22 Fix infinite loops in the example.
 * </pre>
 *
 ****************************************************************************/
#include "xaxicdma.h"
#include "xdebug.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xparameters.h"
#include "xil_util.h"

#include "xinterrupt_wrap.h"
#include "xaxicdma_example.h"

#ifndef __MICROBLAZE__
#include "xpseudo_asm_gcc.h"
#endif

#ifdef __aarch64__
#include "xreg_cortexa53.h"
#include "xil_mmu.h"
#endif


/******************** Constant Definitions **********************************/

#ifndef SDT
#ifndef TESTAPP_GEN
#define DMA_CTRL_DEVICE_ID      XPAR_AXICDMA_0_DEVICE_ID
#endif
#endif

#define BUFFER_BYTESIZE		64	/* Length of the buffers for DMA
					 * transfer
					 */

#define NUMBER_OF_TRANSFERS	4	/* Number of simple transfers to do */
#define POLL_TIMEOUT_COUNTER    1000000U
#define NUMBER_OF_EVENTS	1
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#if (!defined(DEBUG))
extern void xil_printf(const char *format, ...);
#endif


static int DoSimpleTransfer(XAxiCdma *InstancePtr, int Length, int Retries);

static void Example_CallBack(void *CallBackRef, u32 IrqMask, int *IgnorePtr);

#ifndef SDT
int XAxiCdma_SimpleIntrExample(XAxiCdma *InstancePtr, u16 DeviceId);
#else
int XAxiCdma_SimpleIntrExample(XAxiCdma *InstancePtr, UINTPTR BaseAddress);
#endif

/************************** Variable Definitions *****************************/

#ifndef TESTAPP_GEN
static XAxiCdma AxiCdmaInstance;	/* Instance of the XAxiCdma */
#endif

/* Source and Destination buffer for DMA transfer.
 */
volatile static u8 SrcBuffer[BUFFER_BYTESIZE] __attribute__ ((aligned (64)));
volatile static u8 DestBuffer[BUFFER_BYTESIZE] __attribute__ ((aligned (64)));

/* Shared variables used to test the callbacks.
 */
volatile static u32 Done = 0;	/* Dma transfer is done */
volatile static u32 Error = 0;	/* Dma Bus Error occurs */


/*****************************************************************************/
/**
* The entry point for this example. It invokes the example function,
* and reports the execution status.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
* @note		None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{

	int Status;

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Run the interrupt example for simple transfer
	 */
#ifndef SDT
	Status = XAxiCdma_SimpleIntrExample(&AxiCdmaInstance, DMA_CTRL_DEVICE_ID);
#else
	Status = XAxiCdma_SimpleIntrExample(&AxiCdmaInstance, XAXICDMA_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("XAxiCdma_SimpleIntr Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran XAxiCdma_SimpleIntr Example\r\n");
	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
* The example to do the simple transfer through interrupt.
*
* @param	InstancePtr is a pointer to the XAxiCdma instance
* @param	DeviceId/BaseAddress is the Device Id/base address of the
* 		XAxiCdma instance
*
* @return
* 		- XST_SUCCESS if example finishes successfully
* 		- XST_FAILURE if error occurs
*
* @note		If the hardware build has problems with interrupt,
*		then this function hangs
*
******************************************************************************/
#ifndef SDT
int XAxiCdma_SimpleIntrExample(XAxiCdma *InstancePtr, u16 DeviceId)
#else
int XAxiCdma_SimpleIntrExample(XAxiCdma *InstancePtr, UINTPTR BaseAddress)
#endif
{
	XAxiCdma_Config *CfgPtr;
	int Status;
	int SubmitTries = 10;		/* Retry to submit */
	int Tries = NUMBER_OF_TRANSFERS;
	int Index;

	/* Initialize the XAxiCdma device.
	 */
#ifndef SDT
	CfgPtr = XAxiCdma_LookupConfig(DeviceId);
#else
	CfgPtr = XAxiCdma_LookupConfig(BaseAddress);
#endif
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XAxiCdma_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Setup the interrupt system
	 */
	Status = XSetupInterruptSystem(InstancePtr, &XAxiCdma_IntrHandler,
				       CfgPtr->IntrId, CfgPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable all (completion/error/delay) interrupts
	 */
	XAxiCdma_IntrEnable(InstancePtr, XAXICDMA_XR_IRQ_ALL_MASK);

	for (Index = 0; Index < Tries; Index++) {
		Status = DoSimpleTransfer(InstancePtr,
			   BUFFER_BYTESIZE, SubmitTries);

		if(Status != XST_SUCCESS) {
			XDisconnectInterruptCntrl(CfgPtr->IntrId, CfgPtr->IntrParent);
			return XST_FAILURE;
		}
	}

	/* Test finishes successfully, clean up and return
	 */
	XDisconnectInterruptCntrl(CfgPtr->IntrId, CfgPtr->IntrParent);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function does one simple transfer
*
* @param	InstancePtr is a pointer to the XAxiCdma instance
* @param	Length is the transfer length
* @param	Retries is how many times to retry on submission
*
* @return
*		- XST_SUCCESS if transfer is successful
*		- XST_FAILURE if either the transfer fails or the data has
*		  error
*
* @note		None
*
******************************************************************************/
static int DoSimpleTransfer(XAxiCdma *InstancePtr, int Length, int Retries)
{
	u32 Index;
	u8  *SrcPtr;
	u8  *DestPtr;
	int Status;

	Done = 0;
	Error = 0;

	/* Initialize the source buffer bytes with a pattern and the
	 * the destination buffer bytes to zero
	 */
	SrcPtr = (u8 *)SrcBuffer;
	DestPtr = (u8 *)DestBuffer;
	for (Index = 0; Index < Length; Index++) {
		SrcPtr[Index] = Index & 0xFF;
		DestPtr[Index] = 0;
	}

	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)&SrcBuffer, Length);
	Xil_DCacheFlushRange((UINTPTR)&DestBuffer, Length);

	/* Try to start the DMA transfer
	 */
	while (Retries) {
		Retries -= 1;

		Status = XAxiCdma_SimpleTransfer(InstancePtr, (UINTPTR)SrcBuffer,
			(UINTPTR)DestBuffer, Length, Example_CallBack,
			(void *)InstancePtr);

		if (Status == XST_SUCCESS) {
			break;
		}
	}

	if (!Retries) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Failed to submit the transfer with %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Check for any error events to occur */
	Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUMBER_OF_EVENTS, &Error);
	if (Status == XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DMA transfer error\r\n");
		return XST_FAILURE;
	}

	/* Wait until the DMA transfer is done or timeout
	 */
	Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUMBER_OF_EVENTS, &Done);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DMA transfer failed\r\n");
		return XST_FAILURE;
	}

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
	Xil_DCacheInvalidateRange((UINTPTR)&DestBuffer, Length);

	/* Transfer completes successfully, check data
	 *
	 * Compare the contents of destination buffer and source buffer
	 */
	for (Index = 0; Index < Length; Index++) {
		if ( DestPtr[Index] != SrcPtr[Index]) {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "Data check failure %d: %x/%x\r\n",
			    Index, DestPtr[Index], SrcPtr[Index]);
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
* Callback function for the simple transfer. It is called by the driver's
* interrupt handler.
*
* @param	CallBackRef is the reference pointer registered through
*		transfer submission. In this case, it is the pointer to the
* 		driver instance
* @param	IrqMask is the interrupt mask the driver interrupt handler
*		passes to the callback function.
* @param	IgnorePtr is a pointer that is ignored by simple callback
* 		function
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void Example_CallBack(void *CallBackRef, u32 IrqMask, int *IgnorePtr)
{

	if (IrqMask & XAXICDMA_XR_IRQ_ERROR_MASK) {
		Error = TRUE;
	}

	if (IrqMask & XAXICDMA_XR_IRQ_IOC_MASK) {
		Done = TRUE;
	}

}

