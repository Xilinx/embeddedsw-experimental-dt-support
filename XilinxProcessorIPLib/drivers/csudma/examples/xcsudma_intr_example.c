/******************************************************************************
* Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcsudma_intr_example.c
*
* This file contains a design example using the XCsuDma driver in interrupt
* mode. It sends data and expects to receive the same data through the device
* using the local loop back mode.
*
* @note
* The example contains an infinite loop such that if interrupts are not
* working it may hang.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   vnsld  22/10/14 First release
* 1.2   adk    11/22/17 Added peripheral test app support.
* 1.4   adk     04/12/17 Added support for PMC DMA.
* 	adk    11/01/18 Declared static array rather than hard code memory for
*			buffers.
*	adk    18/01/18 Remove unnecessary column in XIntc_Connect() API.
* 1.5   adk    09/05/19 Added volatile keyword for DstDone variable to disable
*			optimizations.
* 1.6   hk     11/18/19 Correct Versal INTR definition.
* 1.9	sk     12/23/20 Add the documentation for XCsuDma_IntrExample() function
* 			parameters to fix the doxygen warning.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsudma.h"
#ifndef SDT
#include "xparameters.h"
#else
#include "xcsudma_example.h"
#endif
#include "xil_exception.h"
#include "xinterrupt_wrap.h"

/************************** Function Prototypes ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define CSUDMA_DEVICE_ID  XPAR_XCSUDMA_0_DEVICE_ID /* CSU DMA device Id */
#endif

#define CSU_SSS_CONFIG_OFFSET	0x008		/**< CSU SSS_CFG Offset */
#define CSUDMA_LOOPBACK_CFG	0x00000050	/**< LOOP BACK configuration
						  *  macro */
#define PMC_SSS_CONFIG_OFFSET	0x500		/**< CSU SSS_CFG Offset */
#define PMCDMA0_LOOPBACK_CFG	0x0000000D	/**< LOOP BACK configuration
						  *  macro for PMCDMA0*/
#define PMCDMA1_LOOPBACK_CFG	0x00000090	/**< LOOP BACK configuration
						  *  macro for PMCDMA1*/
#define SIZE		0x100		/**< Size of the data to be
					  *  transfered */
#if defined(__ICCARM__)
	#pragma data_alignment = 64
	u32 DstBuf[SIZE]; /**< Destination buffer */
	#pragma data_alignment = 64
	u32 SrcBuf[SIZE]; /**< Source buffer */
#else
u32 DstBuf[SIZE] __attribute__ ((aligned (64)));	/**< Destination buffer */
u32 SrcBuf[SIZE] __attribute__ ((aligned (64)));	/**< Source buffer */
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#ifndef SDT
int XCsuDma_IntrExample(XCsuDma *CsuDmaInstance, u16 DeviceId);
#else
int XCsuDma_IntrExample(XCsuDma *CsuDmaInstance, UINTPTR BaseAddress);
#endif
void IntrHandler(void *CallBackRef);

static void SrcHandler(void *CallBackRef, u32 Event);
static void DstHandler(void *CallBackRef, u32 Event);

/************************** Variable Definitions *****************************/

#ifndef TESTAPP_GEN
XCsuDma CsuDma;		/**<Instance of the Csu_Dma Device */
#endif
volatile u32 DstDone = 0;

#ifndef TESTAPP_GEN
/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/* Run the selftest example */
#ifndef SDT
	Status = XCsuDma_IntrExample(&CsuDma, (u16)CSUDMA_DEVICE_ID);
#else
	Status = XCsuDma_IntrExample(&CsuDma, XCSUDMA_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("CSU_DMA Interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CSU_DMA Interrupt Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function performs data transfer in loop back mode in interrupt mode
* and verify the data.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC.
* @param	CsuDmaInstance contains a pointer to the CSU DMA instance
* 		which is going to be connected to the interrupt controller.
* @param	DeviceId is the XPAR_<CSUDMA Instance>_DEVICE_ID macro value.
* @param	IntrId is the interrupt Id and is typically
* 		XPAR_<CSUDMA_instance>_INTR macro value.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
int XCsuDma_IntrExample(XCsuDma *CsuDmaInstance, u16 DeviceId)
#else
int XCsuDma_IntrExample(XCsuDma *CsuDmaInstance, UINTPTR BaseAddress)
#endif
{
	int Status;
	XCsuDma_Config *Config;
	u32 Index = 0;
	u32 *SrcPtr = SrcBuf;
	u32 *DstPtr = DstBuf;
	u32 Test_Data = 0xABCD1234;
	u32 *Ptr = SrcBuf;
	u32 EnLast = 0;
	/*
	 * Initialize the CsuDma driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
#ifndef SDT
	Config = XCsuDma_LookupConfig(DeviceId);
#else
	Config = XCsuDma_LookupConfig(BaseAddress);
#endif
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XCsuDma_CfgInitialize(CsuDmaInstance, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#if defined (versal)
	if (Config->DmaType != XCSUDMA_DMATYPEIS_CSUDMA)
		XCsuDma_PmcReset(Config->DmaType);
#endif

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCsuDma_SelfTest(CsuDmaInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect to the interrupt controller.
	 */
	Status = XSetupInterruptSystem(CsuDmaInstance, &IntrHandler,
			               CsuDmaInstance->Config.IntrId,
				       CsuDmaInstance->Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* Enable interrupts */
	XCsuDma_EnableIntr(CsuDmaInstance, XCSUDMA_DST_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);
	/*
	 * Setting CSU_DMA in loop back mode.
	 */

	if (Config->DmaType == XCSUDMA_DMATYPEIS_CSUDMA) {
		Xil_Out32(XCSU_BASEADDRESS + CSU_SSS_CONFIG_OFFSET,
			((Xil_In32(XCSU_BASEADDRESS + CSU_SSS_CONFIG_OFFSET) & 0xF0000) |
						CSUDMA_LOOPBACK_CFG));
#if defined (versal)
	} else if(Config->DmaType == XCSUDMA_DMATYPEIS_PMCDMA0) {
		Xil_Out32(XPS_PMC_GLOBAL_BASEADDRESS + PMC_SSS_CONFIG_OFFSET,
			((Xil_In32(XPS_PMC_GLOBAL_BASEADDRESS + PMC_SSS_CONFIG_OFFSET) & 0xFF000000) |
						PMCDMA0_LOOPBACK_CFG));
	} else {
		Xil_Out32(XPS_PMC_GLOBAL_BASEADDRESS + PMC_SSS_CONFIG_OFFSET,
			((Xil_In32(XPS_PMC_GLOBAL_BASEADDRESS + PMC_SSS_CONFIG_OFFSET) & 0xFF000000) |
						PMCDMA1_LOOPBACK_CFG));
#endif
	}

	/* Data writing at source address location */

	for(Index = 0; Index < SIZE; Index++)
	{
		*Ptr = Test_Data;
		Test_Data += 0x1;
		Ptr++;
	}

	/* Data transfer in loop back mode */
	XCsuDma_Transfer(CsuDmaInstance, XCSUDMA_DST_CHANNEL, (UINTPTR)DstBuf, SIZE, EnLast);
	XCsuDma_Transfer(CsuDmaInstance, XCSUDMA_SRC_CHANNEL, (UINTPTR)SrcBuf, SIZE, EnLast);

	/* Wait for generation of destination work is done */
	while(DstDone == 0);
	/* Disable interrupts */
	XCsuDma_DisableIntr(CsuDmaInstance, XCSUDMA_DST_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);
	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(CsuDmaInstance, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(CsuDmaInstance, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	/*
	 * Verifying data of transfered by comparing data at
	 * source and address locations.
	 */

	for (Index = 0; Index < SIZE; Index++) {
		if (*SrcPtr != *DstPtr) {
			return XST_FAILURE;
		}
		else {
			SrcPtr++;
			DstPtr++;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the CSU_DMA driver.
*
* This handler reads the interrupt status from the Status register, determines
* the source of the interrupts, calls according callbacks, and finally clears
* the interrupts.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void IntrHandler(void *CallBackRef)
{
	u32 SrcPending;
	u32 DstPending;
	XCsuDma *XCsuDmaPtr = NULL;
	XCsuDmaPtr = (XCsuDma *)((void *)CallBackRef);

	/* Handling interrupt */

	/* Getting pending interrupts of source */
	SrcPending = XCsuDma_IntrGetStatus(XCsuDmaPtr, XCSUDMA_SRC_CHANNEL);
	XCsuDma_IntrClear(XCsuDmaPtr, XCSUDMA_SRC_CHANNEL, SrcPending);
	SrcPending &= (~XCsuDma_GetIntrMask(XCsuDmaPtr, XCSUDMA_SRC_CHANNEL));

	/* Getting pending interrupts of destination */
	DstPending = XCsuDma_IntrGetStatus(XCsuDmaPtr, XCSUDMA_DST_CHANNEL);
	XCsuDma_IntrClear(XCsuDmaPtr, XCSUDMA_DST_CHANNEL, DstPending);
	DstPending &= (~XCsuDma_GetIntrMask(XCsuDmaPtr, XCSUDMA_DST_CHANNEL));


	if (SrcPending != 0x00) {
		SrcHandler(XCsuDmaPtr, SrcPending);
	}

	if (DstPending != 0x00) {
		DstHandler(XCsuDmaPtr, DstPending);
	}
}

/*****************************************************************************/
/**
* This is static function which handlers source channel interrupts.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param	Event specifies which interrupts were occured.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SrcHandler(void *CallBackRef, u32 Event)
{
	if (Event & XCSUDMA_IXR_INVALID_APB_MASK) {
		/*
		 * Code to handle Invalid APB access
		 * Interrupt should be put here.
		 */
	}

	if (Event & XCSUDMA_IXR_FIFO_THRESHHIT_MASK) {
		/*
		 * Code to handle FIFO Threshold hit
		 * Interrupt should be put here.
		 */
	}

	if (Event & (XCSUDMA_IXR_TIMEOUT_MEM_MASK |
				XCSUDMA_IXR_TIMEOUT_STRM_MASK)) {
		/*
		 * Code to handle Timeout
		 * Interrupt should be put here.
		 */
	}
	if (Event & XCSUDMA_IXR_AXI_WRERR_MASK) {
		/*
		 * Code to handle AXI read error
		 * Interrupt should be put here.
		 */
	}
	if (Event & XCSUDMA_IXR_DONE_MASK) {
		/*
		 * Code to handle Done
		 * Interrupt should be put here.
		 */
	}
	if (Event & XCSUDMA_IXR_MEM_DONE_MASK) {
		/*
		 * Code to handle Memory done
		 * Interrupt should be put here.
		 */
	}
}

/*****************************************************************************/
/**
* This static function handles destination channel interrupts.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param	Event specifies which interrupts were occured.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DstHandler(void *CallBackRef, u32 Event)
{
	if (Event & XCSUDMA_IXR_FIFO_OVERFLOW_MASK) {
		/*
		 * Code to handle FIFO overflow
		 * Interrupt should be put here.
		 */
	}
	if (Event & XCSUDMA_IXR_INVALID_APB_MASK) {
		/*
		 * Code to handle Invalid APB access
		 * Interrupt should be put here.
		 */
	}

	if (Event & XCSUDMA_IXR_FIFO_THRESHHIT_MASK) {
		/*
		 * Code to handle FIFO Threshold hit
		 * Interrupt should be put here.
		 */
	}

	if (Event & (XCSUDMA_IXR_TIMEOUT_MEM_MASK |
				XCSUDMA_IXR_TIMEOUT_STRM_MASK)) {
		/*
		 * Code to handle Time out memory or stream
		 * Interrupt should be put here.
		 */
	}
	if (Event & XCSUDMA_IXR_AXI_WRERR_MASK) {
		/*
		 * Code to handle AXI read error
		 * Interrupt should be put here.
		 */
	}
	if (Event & XCSUDMA_IXR_DONE_MASK) {

		DstDone = 1;
	}
}
