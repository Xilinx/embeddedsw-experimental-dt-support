/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xzdma_readonlymode_example.c
*
* This file contains the example using XZDma driver to do simple data read
* on ZDMA device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   vns     2/27/15  First release
*       ms      04/05/17 Modified comment lines notation in functions to
*                        avoid unnecessary description to get displayed
*                        while generating doxygen and also changed filename
*                        tag to include the file in doxygen examples.
* 1.3   mus    08/14/17  Do not perform cache operations if CCI is enabled
* 1.4   adk    11/02/17  Updated example to fix compilation errors for IAR
*			 compiler.
* 1.13	sk     08/02/21	 Make Done variable as volatile to fix failure at
* 			 optimization level 2.
* 1.16  sa     09/29/22  Fix infinite loops in the example.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xzdma.h"
#include "xparameters.h"
#include "xscugic.h"
#include "xil_util.h"
#include "xinterrupt_wrap.h"

/************************** Function Prototypes ******************************/

#ifndef SDT
int XZDma_SimpleReadOnlyExample(u16 DeviceId);
#else
int XZDma_SimpleReadOnlyExample(UINTPTR BaseAddress);
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

#define SIZE 			100	/**< Size of the data to be
					  *  transferred */
#define POLL_TIMEOUT_COUNTER    1000000U
#define NUM_OF_EVENTS           1
/**************************** Type Definitions *******************************/


/************************** Variable Definitions *****************************/

XZDma ZDma;		/**<Instance of the ZDMA Device */
u32 SrcBuf[256];	/**< Source buffer */
volatile static u32 Done = 0;	/**< Done flag */

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

	/* Run the simple read only example */
#ifndef SDT
	Status = XZDma_SimpleReadOnlyExample((u16)ZDMA_DEVICE_ID);
#else
	Status = XZDma_SimpleReadOnlyExample(XPAR_XZDMA_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("ZDMA Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran ZDMA Read Only Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a test of the data transfer in simple mode of read only
*  mode on the ZDMA driver.
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
int XZDma_SimpleReadOnlyExample(u16 DeviceId)
#else
int XZDma_SimpleReadOnlyExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XZDma_Config *Config;
	XZDma_DataConfig Configur; /* Configuration values */
	XZDma_Transfer Data;
	u32 Value;
	u32 Index;

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
	/* Filling the buffer for data transfer */
	Value = 0xABCD1230;
	for (Index = 0; Index < 256; Index++) {
		SrcBuf[Index] = Value++;
	}
	/*
	 * Flushing source address in cache
	 */
	if (!Config->IsCacheCoherent) {
	Xil_DCacheFlushRange((INTPTR)SrcBuf, SIZE);
	}

	/* ZDMA has set in simple transfer of Read only mode */
	Status = XZDma_SetMode(&ZDma, FALSE, XZDMA_RDONLY_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XZDma_SetCallBack(&ZDma, XZDMA_HANDLER_DONE,
			 (void *)DoneHandler, &ZDma);
	/*
	 * Connect to the interrupt controller.
	 */
	Status = XSetupInterruptSystem(&ZDma, &XZDma_IntrHandler,
				       Config->IntrId, Config->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
			return XST_FAILURE;
	}

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

	/* Enable required interrupts */
	XZDma_EnableIntr(&ZDma, XZDMA_IXR_DMA_DONE_MASK);
	/*
	 * Transfer elements
	 */
	Data.DstAddr = 0;
	Data.DstCoherent = 0;
	Data.Pause = 0;
	Data.SrcAddr = (UINTPTR)SrcBuf;
	Data.SrcCoherent = 0;
	Data.Size = SIZE; /* Size in bytes */
	if (Config->IsCacheCoherent) {
		Data.DstCoherent = 1;
		Data.SrcCoherent = 1;
	}

	XZDma_Start(&ZDma, &Data, 1); /* Initiates the data transfer */

	/* Wait till DMA Source done interrupt generated or timeout */
	Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUM_OF_EVENTS, &Done);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Validation of read only mode cannot be performed as it will not
	 * store the values
	 */

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
* This static function handles ZDMA Done interrupts.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
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
