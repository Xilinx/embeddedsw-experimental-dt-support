/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xxxvethernet_example_intr_mcdma.c
*
* Implements examples that utilize the Xxv Ethernet's interrupt driven MCDMA
* packet transfer mode to send and receive frames.
*
* This example demonstrates how to perform a simple send and receive.
*
* Functional guide to example:
*
* - XxvEthernetSgDmaIntrSingleFrameExample demonstrates the simplest way to
*   send and receive frames in interrupt driven mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   hk   06/16/17 First release
* 1.4   rsp  05/12/20 Fix Wint-conversion gcc warning in XxvEthernetSgDmaIntrExample().
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xxxvethernet_example.h"
#include "xxvethernet_example.h"
#include "xinterrupt_wrap.h"
#include "xmcdma.h"
#include "xil_cache.h"
#include "xil_exception.h"
#include "stdio.h"		/* stdio */
#include "stdlib.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif

#if defined(__aarch64__)
#include "xil_mmu.h"
#endif

/*************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#ifndef TESTAPP_GEN
#define XXVETHERNET_DEVICE_ID	XPAR_XXVETHERNET_0_DEVICE_ID
#define AXIMCDMA_DEVICE_ID	XPAR_MCDMA_0_DEVICE_ID
#endif
#endif

#define RXBD_CNT			1024	/* Number of RxBDs to use */
#define TXBD_CNT			1024	/* Number of TxBDs to use */
#define BD_ALIGNMENT			64	/* Byte alignment of BDs */
#define PAYLOAD_SIZE			8900 	/* Payload size used in examples */

#define IP_VERSION_4			0x800	   /* For IPV4, the Ethernet frame
						    * type/length field will have a
						    * value of 0x800
						    */
#define IP_HEADER_VERSION		0x04	   /* For IPv4, the version entry in
						    * IP header is always 4
						    */
#define IP_HEADER_LEN_IN_WORDS		0x05	   /* For our case, the header length
						    * is always 20 bytes (5 words
						    */
#define IP_HEADER_LENGTH		20	   /* IP header length in bytes. Used
						    * as offset to kigure out the start
						    * of TCP header.
						    */
#define IP_HEADER_PROTOCOL_TCP		0x6	   /* IP header protocol entry. For TCP
						    * packets, it is 6.
						    */
#define IP_HEADER_PROTOCOL_UDP		0x17 	   /* IP header protocol entry. For UDP
						    * packets, it is 17.
						    */
#define TCP_HEADER_LEN_IN_WORDS		0x5	   /* For our case, the header length
						    * is always 20 bytes (5 words)
						    */
#define TCP_HEADER_LENGTH		20	   /* IP header length in bytes. Used
						    * as offset to kigure out the start
						    * of TCP header.
						    */
/*
 * Number of bytes to reserve for BD space for the number of BDs desired
 */
#define RXBD_SPACE_BYTES (RXBD_CNT * 64 * 16)
#define TXBD_SPACE_BYTES (TXBD_CNT * 64 * 16)

#define BLOCK_SIZE_2MB 0x200000U

/*************************** Variable Definitions ****************************/

static EthernetFrame TxFrame;	/* Transmit buffer */
static EthernetFrame RxFrame;	/* Receive buffer */

XXxvEthernet XxvEthernetInstance;
XMcdma DmaInstance;

/*
 * Aligned memory segments to be used for buffer descriptors
 */
char RxBdSpace[RXBD_SPACE_BYTES] __attribute__ ((aligned(BD_ALIGNMENT)));
char TxBdSpace[TXBD_SPACE_BYTES] __attribute__ ((aligned(BD_ALIGNMENT)));
char MacAddr[6];

/*
 * Counters to be incremented by callbacks
 */
volatile int FramesRx;	/* Num of frames that have been received */
volatile int FramesTx;	/* Num of frames that have been sent */
volatile int DeviceErrors;/* Num of errors detected in the device */

/*************************** Function Prototypes *****************************/

/*
 * Examples
 */
#ifndef SDT
int XxvEthernetSgDmaIntrExample(INTC *IntcInstancePtr,
				XXxvEthernet *XxvEthernetInstancePtr,
				XMcdma *DmaInstancePtr,
				u16 XxvEthernetDeviceId,
				u16 AxiMcDmaDeviceId);
#else
int XxvEthernetSgDmaIntrExample(XXxvEthernet *XxvEthernetInstancePtr,
				XMcdma *DmaInstancePtr,
				UINTPTR XxvEthernetBaseAddr);
#endif

int XxvEthernetSgDmaIntrSingleFrameExample(XXxvEthernet *XxvEthernetInstancePtr,
					   XMcdma *DmaInstancePtr, u8 ChanId);
static int RxBdSetup(XMcdma *McDmaInstPtr, XXxvEthernet *XxvEthernetInstancePtr,
		XXxvEthernet_Config *MacCfgPtr);
static int TxBdSetup(XMcdma *McDmaInstPtr, XXxvEthernet *XxvEthernetInstancePtr,
		XXxvEthernet_Config *MacCfgPtr);
static void DoneHandler(void *CallBackRef, u32 Chan_id);
static void ErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask);
static void TxDoneHandler(void *CallBackRef, u32 Chan_id);
static void TxErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask);
static int inline AxiEnetMapper(u8 ChanId);
/*****************************************************************************/
/**
*
* This is the main function for the Xxv Ethernet example. This function is not
* included if the example is generated from the TestAppGen test  tool.
*
* @param	None.
*
* @return	- XST_SUCCESS to indicate success.
*		- XST_FAILURE to indicate failure
*
* @note		None.
*
****************************************************************************/
int main(void)
{
	int Status;

	XxvEthernetUtilErrorTrap("\r\n--- Enter main() ---");
	XxvEthernetUtilErrorTrap("This test may take several minutes to finish");
	/*
	 * Call the Xxv Ethernet MCDMA interrupt example , specify the
	 * parameters generated in xparameters.h.
	 */
#ifndef SDT
	Status = XxvEthernetSgDmaIntrExample(&IntcInstance,
					     &XxvEthernetInstance,
					     &DmaInstance,
					     XXVETHERNET_DEVICE_ID,
					     AXIMCDMA_DEVICE_ID);
#else
	Status = XxvEthernetSgDmaIntrExample(&XxvEthernetInstance,
					     &DmaInstance,
					     XXXVETHERNET_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		XxvEthernetUtilErrorTrap("Failed test intr mcdma");
		XxvEthernetUtilErrorTrap("--- Exiting main() ---");
		return XST_FAILURE;
	}

	XxvEthernetUtilErrorTrap("Test passed");
	XxvEthernetUtilErrorTrap("--- Exiting main() ---");

	return XST_SUCCESS;

}


/*****************************************************************************/
/**
*
* This function demonstrates the usage usage of the Xxv Ethernet by sending
* and receiving frames in interrupt driven MCDMA mode.
*
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc
*		component.
* @param	XxvEthernetInstancePtr is a pointer to the instance of the
*		XxvEthernet component.
* @param	DmaInstancePtr is a pointer to the instance of the AXI MCDMA
*		component.
* @param	XxvEthernetDeviceId is Device ID of the Xxv Ethernet Device ,
*		typically XPAR_<XXVETHERNET_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	AxiMcDmaDeviceId is Device ID of the Axi MCDMA Device ,
*		typically XPAR_<AXIMCDMA_instance>_DEVICE_ID value from
*		xparameters.h.
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		AxiMcdma hardware must be initialized before initializing
*		XxvEthernet. Since AxiMcdma reset line is connected to the
*		XxvEthernet reset line, a reset of AxiMcdma hardware during its
*		initialization would reset XxvEthernet.
*
******************************************************************************/
#ifndef SDT
int XxvEthernetSgDmaIntrExample(INTC *IntcInstancePtr,
				XXxvEthernet *XxvEthernetInstancePtr,
				XMcdma *DmaInstancePtr,
				u16 XxvEthernetDeviceId,
				u16 AxiMcDmaDeviceId)
#else
int XxvEthernetSgDmaIntrExample(XXxvEthernet *XxvEthernetInstancePtr,
				XMcdma *DmaInstancePtr,
				UINTPTR XxvEthernetBaseAddr)
#endif
{
	int Status;
	XXxvEthernet_Config *MacCfgPtr;
	XMcdma_Config* DmaConfig;
	UINTPTR XxvDevBaseAddress;
	u8 ChanId;

	/*************************************/
	/* Setup device for first-time usage */
	/*************************************/

	/*
	 *  Get the configuration of XxvEthernet hardware.
	 */
#ifndef SDT
	MacCfgPtr = XXxvEthernet_LookupConfig(XxvEthernetDeviceId);
#else
	MacCfgPtr = XXxvEthernet_LookupConfig(XxvEthernetBaseAddr);
#endif

	XxvDevBaseAddress = MacCfgPtr->XxvDevBaseAddress & XXV_AXIBASEADDR_MASK;

	/*
	 * This example assumes a target MCDMA is connected to XXV Ethernet
	 */

#ifndef SDT
	DmaConfig = XMcdma_LookupConfig(AxiMcDmaDeviceId);
#else
	DmaConfig = XMcdma_LookupConfig(XxvDevBaseAddress);
#endif
	/*
	 * Initialize AXIMCDMA engine. AXIMCDMA engine must be initialized before
	 * XxvEthernet. During AXIMCDMA engine initialization, AXIMCDMA hardware is
	 * reset, and since AXIMCDMA reset line is connected to XxvEthernet, this
	 * would ensure a reset of XxvEthernet.
	 */

	Status = XMcDma_CfgInitialize(DmaInstancePtr, DmaConfig);
	if(Status != XST_SUCCESS) {
		XxvEthernetUtilErrorTrap("Error initializing DMA\r\n");
		return XST_FAILURE;
	}

	/*
	 * Initialize XxvEthernet hardware.
	 */
	Status = XXxvEthernet_CfgInitialize(XxvEthernetInstancePtr, MacCfgPtr,
					MacCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		XxvEthernetUtilErrorTrap("Error in initialize");
		return XST_FAILURE;
	}

	/* When the address < 4GB, the following API sets memory attributes
	 * for a 2MB block. Else the attributes are set for a 1 GB block.
	 * Hence, when the required BD space exceeds 2MB,
	 * call the API for two continuous 2MB blocks.
	 */
#if defined(__aarch64__)
	Xil_SetTlbAttributes((UINTPTR)TxBdSpace, NORM_NONCACHE | INNER_SHAREABLE);
	Xil_SetTlbAttributes((UINTPTR)TxBdSpace + BLOCK_SIZE_2MB, NORM_NONCACHE | INNER_SHAREABLE);
	Xil_SetTlbAttributes((UINTPTR)RxBdSpace, NORM_NONCACHE | INNER_SHAREABLE);
	Xil_SetTlbAttributes((UINTPTR)RxBdSpace + BLOCK_SIZE_2MB, NORM_NONCACHE | INNER_SHAREABLE);
#endif

	RxBdSetup(DmaInstancePtr, XxvEthernetInstancePtr, MacCfgPtr);
	TxBdSetup(DmaInstancePtr, XxvEthernetInstancePtr, MacCfgPtr);

	/* This functions sets Ethernet into local loopback mode
	 * To test in external HW loopback mode, comment this function call
	 * and loop TX and RX externally on your setup.
	 */
	XxvEthernetUtilEnterLocalLoopback(XxvEthernetInstancePtr);
	/****************************/
	/* Run through the examples */
	/****************************/

	for (ChanId = 1 ; ChanId <= DmaConfig->TxNumChannels; ChanId++) {

		/*
		 * Run the XxvEthernet DMA Single Frame Interrupt example
		 */

		Status = XxvEthernetSgDmaIntrSingleFrameExample(XxvEthernetInstancePtr,
							  DmaInstancePtr, ChanId);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/*
	 * Stop the device
	 */
	XXxvEthernet_Stop(XxvEthernetInstancePtr);

	return XST_SUCCESS;
}

static int RxBdSetup(XMcdma *McDmaInstPtr, XXxvEthernet *XxvEthernetInstancePtr,
					 XXxvEthernet_Config *MacCfgPtr)
{
	XMcdma_ChanCtrl *Rx_Chan;
	u8 ChanId;
	int BdCount = RXBD_CNT;
	int Status;
	UINTPTR RxBdSpacePtr;
	int Num_channels;


	RxBdSpacePtr = (UINTPTR)&RxBdSpace;
	Num_channels = McDmaInstPtr->Config.RxNumChannels;

	for (ChanId = 1; ChanId <= McDmaInstPtr->Config.RxNumChannels; ChanId++) {
		Rx_Chan = XMcdma_GetMcdmaRxChan(McDmaInstPtr, ChanId);

		/* Disable all interrupts */
		XMcdma_IntrDisable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);

		Status = XMcDma_ChanBdCreate(Rx_Chan, (UINTPTR)RxBdSpacePtr, BdCount);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx bd create failed with %d\r\n", Status);
			return XST_FAILURE;
		}

		RxBdSpacePtr += (RXBD_CNT * sizeof(XMcdma_Bd));
		/* Setup Interrupt System and register callbacks */
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_HANDLER_DONE,
		                          (void *)DoneHandler, McDmaInstPtr);
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_HANDLER_ERROR,
		                          (void *)ErrorHandler, McDmaInstPtr);
		Status = XSetupInterruptSystem(McDmaInstPtr, &XMcdma_IntrHandler,
					       McDmaInstPtr->Config.IntrId[Num_channels+(ChanId-1)],
					       McDmaInstPtr->Config.IntrParent,
					       XINTERRUPT_DEFAULT_PRIORITY);
		if (Status != XST_SUCCESS) {
		      xil_printf("Failed RX interrupt setup %d\r\n", ChanId);
		      return XST_FAILURE;
		}
	 }

	return XST_SUCCESS;
}

static int TxBdSetup(XMcdma *McDmaInstPtr, XXxvEthernet *XxvEthernetInstancePtr,
					 XXxvEthernet_Config *MacCfgPtr)
{
	XMcdma_ChanCtrl  *Tx_Chan;
	u8 ChanId;
	int BdCount = RXBD_CNT;
	int Status;
	UINTPTR TxBdSpacePtr;

	TxBdSpacePtr = (UINTPTR)&TxBdSpace;

	for (ChanId = 1; ChanId <= McDmaInstPtr->Config.TxNumChannels; ChanId++) {
		Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, ChanId);

		XMcdma_IntrDisable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);

		Status = XMcDma_ChanBdCreate(Tx_Chan, (UINTPTR)TxBdSpacePtr, BdCount);
		if (Status != XST_SUCCESS) {
			xil_printf("TX bd create failed with %d\r\n", Status);
			return XST_FAILURE;
		}

		TxBdSpacePtr += (TXBD_CNT * sizeof(XMcdma_Bd));
		/* Setup Interrupt System and register callbacks */
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_TX_HANDLER_DONE,
				   (void *)TxDoneHandler, McDmaInstPtr);
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_TX_HANDLER_ERROR,
				   (void *)TxErrorHandler, McDmaInstPtr);

		Status = XSetupInterruptSystem(McDmaInstPtr, &XMcdma_TxIntrHandler,
					       McDmaInstPtr->Config.IntrId[ChanId - 1],
					       McDmaInstPtr->Config.IntrParent,
					       XINTERRUPT_DEFAULT_PRIORITY);
		if (Status != XST_SUCCESS) {
		      xil_printf("Failed TX interrupt setup %d\r\n", ChanId);
		      return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

void AxiEthernetSetMAC(char *MacAddr, u8 ChanTdest)
{
	int i;

	for (i = 0 ; i < XXE_MAC_ADDR_SIZE ; i++) {
		MacAddr[i] = DestAddr[i];
		if (i == 1)
			MacAddr[i] = ChanTdest;
	}
}

static int inline AxiEnetMapper(u8 ChanId)
{

	switch(ChanId) {
	case 1:
		AxiEthernetSetMAC(MacAddr, 0x00);
		return XST_SUCCESS;
	case 2:
		AxiEthernetSetMAC(MacAddr, 0x0f);
		return XST_SUCCESS;
	case 3:
		AxiEthernetSetMAC(MacAddr, 0x10);
		return XST_SUCCESS;
	case 4:
		AxiEthernetSetMAC(MacAddr, 0x18);
		return XST_SUCCESS;
	case 5:
		AxiEthernetSetMAC(MacAddr, 0x20);
		return XST_SUCCESS;
	case 6:
		AxiEthernetSetMAC(MacAddr, 0x28);
		return XST_SUCCESS;
	case 7:
		AxiEthernetSetMAC(MacAddr, 0x30);
		return XST_SUCCESS;
	case 8:
		AxiEthernetSetMAC(MacAddr, 0x38);
		return XST_SUCCESS;
	case 9:
		AxiEthernetSetMAC(MacAddr, 0x40);
		return XST_SUCCESS;
	case 10:
		AxiEthernetSetMAC(MacAddr, 0x48);
		return XST_SUCCESS;
	case 11:
		AxiEthernetSetMAC(MacAddr, 0x50);
		return XST_SUCCESS;
	case 12:
		AxiEthernetSetMAC(MacAddr, 0x58);
		return XST_SUCCESS;
	case 13:
		AxiEthernetSetMAC(MacAddr, 0x60);
		return XST_SUCCESS;
	case 14:
		AxiEthernetSetMAC(MacAddr, 0x68);
		return XST_SUCCESS;
	case 15:
		AxiEthernetSetMAC(MacAddr, 0x70);
		return XST_SUCCESS;
	case 16:
		AxiEthernetSetMAC(MacAddr, 0x78);
		return XST_SUCCESS;
	}

	return XST_FAILURE;
}

static void DoneHandler(void *CallBackRef, u32 Chan_id)
{
	FramesRx++;
}


static void ErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask)
{
	xil_printf("Inside error Handler Chan_id is %d Error word is 0x%x \n\r", Chan_id, Mask);
}

static void TxDoneHandler(void *CallBackRef, u32 Chan_id)
{
	FramesTx++;
}

static void TxErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask)
{
	xil_printf("Inside Tx error Handler Chan_id is %d and Mask %x\n\r", Chan_id, Mask);
}

/*****************************************************************************/
/**
*
* This function demonstrates the usage of the Xxv Ethernet by sending and
* receiving a single frame in interrupt mode using MCDMA.
* The source packet will be described by two descriptors. It will be received
* into a buffer described by a single descriptor.
*
* @param	XxvEthernetInstancePtr is a pointer to the instance of the
*		Xxv Ethernet component.
* @param	DmaInstancePtr   is a pointer to the instance of the Dma
*		component.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		None.
*
******************************************************************************/
int XxvEthernetSgDmaIntrSingleFrameExample(XXxvEthernet *XxvEthernetInstancePtr,
										   XMcdma *DmaInstancePtr, u8 ChanId)
{
	int Status;
	u32 TxFrameLength;
	u32 RxFrameLength;
	int PayloadSize = PAYLOAD_SIZE;
	XMcdma_ChanCtrl *Rx_Chan, *Tx_Chan;
	XMcdma_Bd *BdCurPtr, *Bd1Ptr;
	u32 BdSts, CrBits, len, i;
	UINTPTR TxBufPtr;

	/*
	 * Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesTx = 0;
	DeviceErrors = 0;
	memset(RxFrame,0,sizeof(RxFrame));
	memset(TxFrame,0,sizeof(TxFrame));

	Rx_Chan = XMcdma_GetMcdmaRxChan(DmaInstancePtr, ChanId);
	Tx_Chan = XMcdma_GetMcdmaTxChan(DmaInstancePtr, ChanId);

	/*
	 * Calculate the frame length (not including FCS)
	 */
	TxFrameLength = XXE_HDR_SIZE + PayloadSize;

	AxiEnetMapper(ChanId);

	xil_printf("ChanId %d, MacAddr: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n\r", ChanId,
			MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]);

	/*
	 * Setup packet to be transmitted
	 */
	XxvEthernetUtilFrameHdrFormatMAC(&TxFrame, MacAddr, MacAddr);
	/* If sending externally */
	/* XxvEthernetUtilFrameHdrFormatMAC(&TxFrame, MacAddr, XxvEthernetMAC); */
	XxvEthernetUtilFrameHdrFormatType(&TxFrame, PayloadSize);
	XxvEthernetUtilFrameSetPayloadData(&TxFrame, PayloadSize);

	/*
	 * Flush the TX frame before giving it to DMA TX channel to transmit.
	 */
	Xil_DCacheFlushRange((UINTPTR)&TxFrame, TxFrameLength);

	/*
	 * Clear out receive packet memory area
	 */
	XxvEthernetUtilFrameMemClear(&RxFrame);

	/*
	 * Invalidate the RX frame before giving it to DMA RX channel to
	 * receive data.
	 */
	Xil_DCacheInvalidateRange((UINTPTR)&RxFrame, TxFrameLength);

	/*
	 * Enable DMA RX interrupt.
	 *
	 * Interrupt coalescing parameters are left at their default settings
	 * which is to interrupt the processor after every frame has been
	 * processed by the DMA engine.
	 */
	XMcdma_IntrEnable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);


	Status = XMcDma_ChanSubmit(Rx_Chan, (UINTPTR)&RxFrame,
				   TxFrameLength);
	if (Status != XST_SUCCESS) {
		xil_printf("ChanSubmit failed\n\r");
		return XST_FAILURE;
	}

	Status = XMcDma_ChanToHw(Rx_Chan);
	if (Status != XST_SUCCESS) {
			xil_printf("XMcDma_ChanToHw failed\n\r");
			return XST_FAILURE;
	}

	/*
	 * Enable DMA transmit interrupts
	 */
	XMcdma_IntrEnable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);

	BdCurPtr = (XMcdma_Bd *)XMcdma_GetChanCurBd(Tx_Chan);
	TxBufPtr = (UINTPTR)&TxFrame;
	for (i = 0 ; i < 2 ; i++) {
		CrBits = 0;

		if (i == 0)
			len = 64;
		if (i == 1)
			len = TxFrameLength - 64;

		Status = XMcDma_ChanSubmit(Tx_Chan, TxBufPtr,
								   len);
		if (Status != XST_SUCCESS) {
			xil_printf("ChanSubmit failed\n\r");
			return XST_FAILURE;
		}
		if (i == 0) {
			CrBits |= XMCDMA_BD_CTRL_SOF_MASK;
		}
		if (i == 1) {
			CrBits |= XMCDMA_BD_CTRL_EOF_MASK;
		}

		XMcDma_BdSetCtrl(BdCurPtr, CrBits);
		XMCDMA_CACHE_FLUSH((UINTPTR)(BdCurPtr));
		TxBufPtr += 64;
		BdCurPtr = (XMcdma_Bd *)XMcdma_BdChainNextBd(Tx_Chan, BdCurPtr);
	}

	Status = XMcDma_ChanToHw(Tx_Chan);
	if (Status != XST_SUCCESS) {
		xil_printf("XMcDma_ChanToHw failed for Tx\n\r");
		return XST_FAILURE;
	}

	/*
	 * Start the device. Transmission commences now!
	 */
	if (XXxvEthernet_Start(XxvEthernetInstancePtr)) {
		return XST_FAILURE;
	}

	/*
	 * Wait for transmission to complete
	 */
	while (!FramesTx);

	/*
	 * Now that the frame has been sent, post process our TxBDs.
	 * Since we have only submitted 2 to HW, then there should be only 2 ready
	 * for post processing.
	 */
	if (XMcdma_BdChainFromHW(Tx_Chan, 2, &Bd1Ptr) == 0) {
		XxvEthernetUtilErrorTrap("TxBDs were not ready for post processing");
		return XST_FAILURE;
	}

	Status = XMcdma_BdChainFree(Tx_Chan, 2, Bd1Ptr);
	if (Status != XST_SUCCESS) {
		XxvEthernetUtilErrorTrap("Error freeing TxBDs");
	}

	/*
	 * Wait for Rx indication
	 */
	while (!FramesRx);


	/*
	 * Now that the frame has been received, post process our RxBD.
	 * Since we have only submitted 1 to HW, then there should be only 1
	 * ready for post processing.
	 */
	if (XMcdma_BdChainFromHW(Rx_Chan, 1, &Bd1Ptr) == 0) {
		XxvEthernetUtilErrorTrap("RxBD was not ready for post processing");
		return XST_FAILURE;
	}

	BdCurPtr = Bd1Ptr;
	BdSts = XMcDma_BdGetSts(BdCurPtr);
	if ((BdSts & XMCDMA_BD_STS_ALL_ERR_MASK) ||
		(!(BdSts & XMCDMA_BD_STS_COMPLETE_MASK))) {
			XxvEthernetUtilErrorTrap("Rx Error");
			return XST_FAILURE;
	}
	else {
		RxFrameLength =  XMcdma_BdRead64((BdCurPtr), XMCDMA_BD_STS_OFFSET)
							& 0x007FFFFF;
	}

	if (RxFrameLength != TxFrameLength) {
		xil_printf("RxFrameLength 0x%x, TxFrameLength 0x%x \n\r", RxFrameLength, TxFrameLength);
		XxvEthernetUtilErrorTrap("Length mismatch");
		return XST_FAILURE;
	}


	if (XxvEthernetUtilFrameVerify(&TxFrame, &RxFrame) != 0) {
		XxvEthernetUtilErrorTrap("Data mismatch");
		return XST_FAILURE;
	}

	Status = XMcdma_BdChainFree(Rx_Chan, 1, Bd1Ptr);
	if (Status != XST_SUCCESS) {
		XxvEthernetUtilErrorTrap("Error freeing TxBDs");
	}

	/*
	 * Finished this example. If everything worked correctly, all TxBDs and
	 * RxBDs should be free for allocation. Stop the device.
	 */
	XXxvEthernet_Stop(XxvEthernetInstancePtr);

	xil_printf("ChanId %d Single frame interrupt example passed \n\r", ChanId);
	return XST_SUCCESS;
}
