/******************************************************************************
* Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xzdma_writeonlymode_example.c
*
* This file contains the example using XZDma driver to do simple data transfer
* in Write only mode on ZDMA device. In this mode data will be predefined
* and will be repetitively written into the given address and for given size.
* For ADMA only 2 words are repeated and for GDMA 4 words are repeated.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   vns     2/27/15  First release
*       ms      04/05/17 Modified comment lines notation in functions to
*                        avoid unnecessary description to get displayed
*                        while generating doxygen.
* 1.3   mus    08/14/17  Do not perform cache operations if CCI is enabled
* 1.4   adk    11/02/17  Updated example to fix compilation errors for IAR
*			 compiler.
* 1.7   adk    18/03/19  Update the example data verification check to support
*			 versal adma IP.
* 1.7   adk    21/03/19  Fix alignment pragmas in the example for IAR compiler.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xzdma.h"
#include "xparameters.h"
#include "xinterrupt_wrap.h"

/************************** Function Prototypes ******************************/

#ifndef SDT
int XZDma_WriteOnlyExample(u16 DeviceId);
#else
int XZDma_WriteOnlyExample(UINTPTR BaseAddress);
#endif
static void DoneHandler(void *CallBackRef);

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define ZDMA_DEVICE_ID		XPAR_XZDMA_0_DEVICE_ID /* ZDMA device Id */
#endif

#define SIZE			1024 /* Size of the data to be written */

/**************************** Type Definitions *******************************/


/************************** Variable Definitions *****************************/

XZDma ZDma;		/**<Instance of the ZDMA Device */
u32 SrcBuf[4];		/**< Source buffer */
#if defined(__ICCARM__)
    #pragma data_alignment = 64
	u32 DstBuf[300]; /**< Destination buffer */
#else
u32 DstBuf[300] __attribute__ ((aligned (64))); /**< Destination buffer */
#endif
u8 Done = 0;		/**< Done Flag for interrupt generation */

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

	/* Run the simple write only mode example */
#ifndef SDT
	Status = XZDma_WriteOnlyExample((u16)ZDMA_DEVICE_ID);
#else
	Status = XZDma_WriteOnlyExample(XPAR_XZDMA_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("ZDMA Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Write Only mode ZDMA Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a test of the data transfer in simple mode of write only
* mode on the ZDMA driver.
*
* @param	DeviceId is the XPAR_<ZDMA Instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
int XZDma_WriteOnlyExample(u16 DeviceId)
#else
int XZDma_WriteOnlyExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XZDma_Config *Config;
	XZDma_DataConfig Configur; /* Configuration values */
	XZDma_Transfer Data;
	u32 *Buf = (u32 *)DstBuf;
	u32 Index;
	u32 Index1;

	/*
	 * Initialize the ZDMA driver so that it's ready to use.
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
#ifndef SDT
	Config = XZDma_LookupConfig(DeviceId);
#else
	Config = XZDma_LookupConfig(BaseAddress);
#endif
	if (NULL == Config) {
		return XST_FAILURE;
	}
	Status = XZDma_CfgInitialize(&ZDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XZDma_SelfTest(&ZDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* ZDMA has set in simple transfer of Normal mode */
	Status = XZDma_SetMode(&ZDma, FALSE, XZDMA_WRONLY_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XZDma_EnableIntr(&ZDma, XZDMA_IXR_DMA_DONE_MASK);
	/*
	 * Connect to the interrupt controller.
	 */
	Status = XSetupInterruptSystem(&ZDma, &XZDma_IntrHandler,
				       Config->IntrId, Config->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
			return XST_FAILURE;
	}
	(void)XZDma_SetCallBack(&ZDma,
		XZDMA_HANDLER_DONE, (void *)(DoneHandler),
							&ZDma);
	/* Configuration settings */
	Configur.OverFetch = 0;
	Configur.SrcIssue = 0x1F;
	Configur.SrcBurstType = XZDMA_INCR_BURST;
	Configur.SrcBurstLen = 0xF;
	Configur.DstBurstType = XZDMA_INCR_BURST;
	Configur.DstBurstLen = 0xF;
	if (Config->IsCacheCoherent) {
		Configur.SrcCache = 0xF;
		Configur.DstCache = 0xF;
	}
	XZDma_SetChDataConfig(&ZDma, &Configur);
	/*
	 * Transfer elements
	 */
	Data.DstAddr = (UINTPTR)DstBuf;
	Data.DstCoherent = 0;
	Data.Pause = 0;
	Data.SrcAddr = (UINTPTR)NULL;
	Data.SrcCoherent = 0;
	Data.Size = SIZE; /* Size in bytes */
	if (Config->IsCacheCoherent) {
		Data.DstCoherent = 1;
		Data.SrcCoherent = 1;
	}

	if (ZDma.Config.DmaType == 0) { /* For GDMA */
		SrcBuf[0] = 0x1234;
		SrcBuf[1] = 0xABCD;
		SrcBuf[2] = 0x4567;
		SrcBuf[3] = 0xEF;
		XZDma_WOData(&ZDma, SrcBuf);
	}
	else { /* For ADMA */
		SrcBuf[0] = 0x1234;
		SrcBuf[1] = 0xABCD;
		XZDma_WOData(&ZDma, SrcBuf);
	}

	if (!Config->IsCacheCoherent) {
	Xil_DCacheInvalidateRange((INTPTR)DstBuf, SIZE);
	}

	XZDma_Start(&ZDma, &Data, 1); /* Initiates the data transfer */

	/* Wait till DMA destination done interrupt generated */
	while (Done == 0);

	/* Validation */
	if (ZDma.Config.DmaType == 0) { /* For GDMA */
		for (Index = 0; Index < (SIZE/4)/4; Index++) {
			for (Index1 = 0; Index1 < 4; Index1++) {
				if (SrcBuf[Index1] != *Buf++) {
					return XST_FAILURE;
				}
			}
		}
	}
	else { /* For ADMA */
#ifdef versal
		for (Index = 0; Index < (SIZE/4)/4; Index++) {
			for (Index1 = 0; Index1 < 4; Index1++) {
#else
		for (Index = 0; Index < (SIZE/4)/2; Index++) {
			for (Index1 = 0; Index1 < 2; Index1++) {
#endif
				if (SrcBuf[Index1] != *Buf++) {
						return XST_FAILURE;
				}
			}
		}
	}

return XST_SUCCESS;

}

/*****************************************************************************/
/**
* This static function handles ZDMA Done interrupts.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param	Event specifies which interrupts were occurred.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DoneHandler(void *CallBackRef)
{
	Done = 1;

}
