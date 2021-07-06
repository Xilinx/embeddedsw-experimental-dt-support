/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xmcdma_interrupt_example.c
 *
 * This file demonstrates how to use the mcdma driver on the Xilinx AXI
 * MCDMA core (AXI MCDMA) to transfer packets in interrupt mode.
 *
 * This examples shows how to do multiple packets and multiple BD's
 * Per packet transfers.
 *
 * H/W Requirements:
 * In order to test this example at the design level AXI MCDMA MM2S should
 * be connected with the S2MM channel.
 *
 * System level Considerations for Zynq UltraScale+ designs:
 * Please refer xmcdma_polled_example.c file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date      Changes
 * ----- ---- --------  -------------------------------------------------------
 * 1.0	 adk  18/07/17	Initial Version.
 * 1.2	 rsp  07/19/18  Read channel count from IP config.
 *			Fix gcc 'pointer from integer without a cast' warning.
 *	 rsp  08/17/18	Fix typos and rephrase comments.
 *	 rsp  08/17/18  Read Length register value from IP config.
 * 1.3   rsp  02/05/19  Remove snooping enable from application.
 *       rsp  02/06/19  Programmatically select cache maintenance ops for HPC
 *                      and non-HPC designs. In Rx remove arch64 specific dsb
 *                      instruction by performing cache invalidate operation
 *                      for all supported architectures.
 * </pre>
 *
 * ***************************************************************************
 */
/***************************** Include Files *********************************/
#include "xmcdma.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xmcdma_hw.h"
#include "xinterrupt_wrap.h"
#include "xmcdma_example.h"

#ifdef __aarch64__
#include "xil_mmu.h"
#endif


/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#ifndef SDT
#define MCDMA_DEV_ID	XPAR_AXI_MCDMA_0_DEVICE_ID


#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif XPAR_MIG7SERIES_0_BASEADDR
#define DDR_BASE_ADDR	XPAR_MIG7SERIES_0_BASEADDR
#elif XPAR_MIG_0_BASEADDR
#define DDR_BASE_ADDR	XPAR_MIG_0_BASEADDR
#elif XPAR_PSU_DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#ifdef XPAR_PSU_DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#ifdef XPAR_PSU_R5_DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_PSU_R5_DDR_0_S_AXI_BASEADDR
#endif

#else

#ifdef XPAR_MEM0_BASEADDRESS
#define DDR_BASE_ADDR		XPAR_MEM0_BASEADDRESS
#endif
#endif

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
			DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x10000000)
#endif

#define TX_BD_SPACE_BASE	(MEM_BASE_ADDR)
#define RX_BD_SPACE_BASE	(MEM_BASE_ADDR + 0x10000000)
#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x20000000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x50000000)

#define NUMBER_OF_BDS_PER_PKT		10
#define NUMBER_OF_PKTS_TO_TRANSFER 	100
#define NUMBER_OF_BDS_TO_TRANSFER	(NUMBER_OF_PKTS_TO_TRANSFER * \
						NUMBER_OF_BDS_PER_PKT)

#define PACKETS_PER_IRQ 50
#define MAX_PKT_LEN		1024
#define BLOCK_SIZE_2MB 0x200000U

#define TEST_START_VALUE	0xC


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
static int RxSetup(XMcdma *McDmaInstPtr);
static int TxSetup(XMcdma *McDmaInstPtr);
static int SendPacket(XMcdma *McDmaInstPtr);
static int CheckData(u8 *RxPacket, int ByteCount);
static void TxDoneHandler(void *CallBackRef, u32 Chan_id);
static void TxErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask);
static void DoneHandler(void *CallBackRef, u32 Chan_id);
static void ErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XMcdma AxiMcdma;

volatile int RxChanDone;
volatile int TxChanDone;
volatile int RxDone;
volatile int TxDone;
volatile int Error;
int num_channels;


/*
 * Buffer for transmit packet. Must be 32-bit aligned to be used by DMA.
 */
UINTPTR *Packet = (UINTPTR *) TX_BUFFER_BASE;

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the tests on DMA core. It sets up
* DMA engine to be ready to receive and send packets, then a packet is
* transmitted and will be verified after it is received via the DMA.
*
* @param	None
*
* @return
*		- XST_SUCCESS if test passes
*		- XST_FAILURE if test fails.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status, i;

	XMcdma_Config *Mcdma_Config;

	xil_printf("\r\n--- Entering main() --- \r\n");

#ifdef __aarch64__
#if (TX_BD_SPACE_BASE < 0x100000000UL)
	for (i = 0; i < (RX_BD_SPACE_BASE - TX_BD_SPACE_BASE) / BLOCK_SIZE_2MB; i++) {
		Xil_SetTlbAttributes(TX_BD_SPACE_BASE + (i * BLOCK_SIZE_2MB), NORM_NONCACHE);
		Xil_SetTlbAttributes(RX_BD_SPACE_BASE + (i * BLOCK_SIZE_2MB), NORM_NONCACHE);
	}
#else
	Xil_SetTlbAttributes(TX_BD_SPACE_BASE, NORM_NONCACHE);
#endif
#endif


#ifndef SDT
	Mcdma_Config = XMcdma_LookupConfig(MCDMA_DEV_ID);
	if (!Mcdma_Config) {
			xil_printf("No config found for %d\r\n", MCDMA_DEV_ID);

			return XST_FAILURE;
	}
#else
	Mcdma_Config = XMcdma_LookupConfig(XMCDMA_BASEADDRESS);
	if (!Mcdma_Config) {
			xil_printf("No config found for %llx\r\n", XMCDMA_BASEADDRESS);

			return XST_FAILURE;
	}
#endif

	Status = XMcDma_CfgInitialize(&AxiMcdma, Mcdma_Config);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Read numbers of channels from IP config */
	num_channels = Mcdma_Config->RxNumChannels;

	Status = TxSetup(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	Status = RxSetup(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Initialize flags */
	RxChanDone = 0;
	TxChanDone  = 0;
	RxDone = 0;
	TxDone = 0;
	Error = 0;

	Status = SendPacket(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed send packet\r\n");
		return XST_FAILURE;
	}

	 while (1) {
	        if ((RxDone >= NUMBER_OF_BDS_TO_TRANSFER * num_channels) && !Error)
	              break;
	 }

	xil_printf("AXI MCDMA SG Interrupt Test %s\r\n",
		(Status == XST_SUCCESS)? "passed":"failed");

	xil_printf("--- Exiting main() --- \r\n");

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up RX channel of the DMA engine to be ready for packet
* reception
*
* @param	McDmaInstPtr is the pointer to the instance of the AXI MCDMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int RxSetup(XMcdma *McDmaInstPtr)
{
	XMcdma_ChanCtrl *Rx_Chan;
	int ChanId;
	int BdCount = NUMBER_OF_BDS_TO_TRANSFER;
	UINTPTR RxBufferPtr;
	UINTPTR RxBdSpacePtr;
	int Status;
	u32 i, j;
	u32 buf_align;

	RxBufferPtr = RX_BUFFER_BASE;
	RxBdSpacePtr = RX_BD_SPACE_BASE;


	for (ChanId = 1; ChanId <= num_channels; ChanId++) {
		Rx_Chan = XMcdma_GetMcdmaRxChan(McDmaInstPtr, ChanId);

		/* Disable all interrupts */
		XMcdma_IntrDisable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);

		Status = XMcDma_ChanBdCreate(Rx_Chan, RxBdSpacePtr, BdCount);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx bd create failed with %d\r\n", Status);
			return XST_FAILURE;
		}

		for (j = 0 ; j < NUMBER_OF_PKTS_TO_TRANSFER; j++) {
			for (i = 0 ; i < NUMBER_OF_BDS_PER_PKT; i++) {
				Status = XMcDma_ChanSubmit(Rx_Chan, RxBufferPtr,
							  MAX_PKT_LEN);
				if (Status != XST_SUCCESS) {
					xil_printf("ChanSubmit failed\n\r");
					return XST_FAILURE;
				}

				/* Clear the receive buffer, so we can verify data */
				memset((void *)RxBufferPtr, 0, MAX_PKT_LEN);

				if(!McDmaInstPtr->Config.IsRxCacheCoherent)
					Xil_DCacheInvalidateRange((UINTPTR)RxBufferPtr, MAX_PKT_LEN);

				RxBufferPtr += MAX_PKT_LEN;
				if (!Rx_Chan->Has_Rxdre) {
					buf_align = RxBufferPtr % 64;
					if (buf_align > 0)
						buf_align = 64 - buf_align;
					RxBufferPtr += buf_align;
				}
			}

			RxBufferPtr += MAX_PKT_LEN;
			if (!Rx_Chan->Has_Rxdre) {
				buf_align = RxBufferPtr % 64;
				if (buf_align > 0)
					buf_align = 64 - buf_align;
				RxBufferPtr += buf_align;
			}
			XMcdma_SetChanCoalesceDelay(Rx_Chan, PACKETS_PER_IRQ, 255);
		}

		Status = XMcDma_ChanToHw(Rx_Chan);
		if (Status != XST_SUCCESS) {
				xil_printf("XMcDma_ChanToHw failed\n\r");
				return XST_FAILURE;
		}

		RxBufferPtr += MAX_PKT_LEN;
		if (!Rx_Chan->Has_Rxdre) {
			buf_align = RxBufferPtr % 64;
			if (buf_align > 0)
				buf_align = 64 - buf_align;
			RxBufferPtr += buf_align;
		}

		RxBdSpacePtr += BdCount * Rx_Chan->Separation;

		/* Setup Interrupt System and register callbacks */
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_HANDLER_DONE,
		                          (void *)DoneHandler, McDmaInstPtr);
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_HANDLER_ERROR,
		                          (void *)ErrorHandler, McDmaInstPtr);

		Status = XSetupInterruptSystem(McDmaInstPtr, &XMcdma_IntrHandler,
					       McDmaInstPtr->Config.IntrId[num_channels+(ChanId-1)],
					       McDmaInstPtr->Config.IntrParent,
					       XINTERRUPT_DEFAULT_PRIORITY);
		if (Status != XST_SUCCESS) {
		      xil_printf("Failed RX interrupt setup %d\r\n", ChanId);
		      return XST_FAILURE;
		}
		XMcdma_IntrEnable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);

	 }

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the TX channel of a DMA engine to be ready for packet
* transmission
*
* @param	McDmaInstPtr is the instance pointer to the AXI MCDMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int TxSetup(XMcdma *McDmaInstPtr)
{
	XMcdma_ChanCtrl *Tx_Chan;
	int ChanId;
	int BdCount = NUMBER_OF_BDS_TO_TRANSFER;
	UINTPTR TxBufferPtr;
	UINTPTR TxBdSpacePtr;
	int Status;
	u32 i, j;
	u32 buf_align;

	TxBufferPtr = TX_BUFFER_BASE;
	TxBdSpacePtr = TX_BD_SPACE_BASE;

	for (ChanId = 1; ChanId <= num_channels; ChanId++) {
		Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, ChanId);

		/* Disable all interrupts */
		XMcdma_IntrDisable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);

		Status = XMcDma_ChanBdCreate(Tx_Chan, TxBdSpacePtr, BdCount);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx bd create failed with %d\r\n", Status);
			return XST_FAILURE;
		}

		for (j = 0 ; j < NUMBER_OF_PKTS_TO_TRANSFER; j++) {
			for (i = 0 ; i < NUMBER_OF_BDS_PER_PKT; i++) {
				Status = XMcDma_ChanSubmit(Tx_Chan, TxBufferPtr,
							   MAX_PKT_LEN);
				if (Status != XST_SUCCESS) {
					xil_printf("ChanSubmit failed\n\r");
					return XST_FAILURE;
				}

				TxBufferPtr += MAX_PKT_LEN;
				if (!Tx_Chan->Has_Txdre) {
					buf_align = TxBufferPtr % 64;
					if (buf_align > 0)
						buf_align = 64 - buf_align;
				    TxBufferPtr += buf_align;
				}

				/* Clear the receive buffer, so we can verify data */
				memset((void *)TxBufferPtr, 0, MAX_PKT_LEN);

			}

			TxBufferPtr += MAX_PKT_LEN;
			if (!Tx_Chan->Has_Txdre) {
				buf_align = TxBufferPtr % 64;
				if (buf_align > 0)
					buf_align = 64 - buf_align;
			    TxBufferPtr += buf_align;
			}
		}

		TxBdSpacePtr += BdCount * Tx_Chan->Separation;
		XMcdma_SetChanCoalesceDelay(Tx_Chan, PACKETS_PER_IRQ, 255);

		/* Setup Interrupt System and register callbacks */
		 XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_TX_HANDLER_DONE,
                            (void *)TxDoneHandler, McDmaInstPtr);
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_TX_HANDLER_ERROR,
                             (void *)TxErrorHandler, McDmaInstPtr);

		Status = XSetupInterruptSystem(McDmaInstPtr, &XMcdma_TxIntrHandler,
					       McDmaInstPtr->Config.IntrId[ChanId-1],
					       McDmaInstPtr->Config.IntrParent,
					       XINTERRUPT_DEFAULT_PRIORITY);
		if (Status != XST_SUCCESS) {
		      xil_printf("Failed Tx interrupt setup %d\r\n", ChanId);
		      return XST_FAILURE;
		}

		XMcdma_IntrEnable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);
	 }


	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function checks data buffer after the DMA transfer is finished.
*
* @param	RxPacket is the pointer to Rx packet.
* @param	ByteCount is the length of RX packet.
*
* @return	- XST_SUCCESS if validation is successful
*		- XST_FAILURE if validation is failure.
*
* @note		None.
*
******************************************************************************/
static int CheckData(u8 *RxPacket, int ByteCount)
{
	u32 Index;
	u8 Value;

	Value = TEST_START_VALUE;

	for(Index = 0; Index < ByteCount; Index++) {
			if (RxPacket[Index] != Value) {
				xil_printf("Data error : %x/%x\r\n",
							(unsigned int)RxPacket[Index],
							(unsigned int)Value);
				return XST_FAILURE;
				break;
			}
			Value = (Value + 1) & 0xFF;
	}


	return XST_SUCCESS;
}

static int SendPacket(XMcdma *McDmaInstPtr)
{
	XMcdma_ChanCtrl *Tx_Chan = NULL;
	u32 Index, Pkts, Index1;
	XMcdma_Bd *BdCurPtr;
	u32 Status;
	u8 *TxPacket;
	u8 Value;
	u32 ChanId;

	BdCurPtr = (XMcdma_Bd *)TX_BD_SPACE_BASE;
	for (ChanId = 1; ChanId <= num_channels; ChanId++) {
		Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, ChanId);

		for(Index = 0; Index < NUMBER_OF_PKTS_TO_TRANSFER; Index++) {
			for(Pkts = 0; Pkts < NUMBER_OF_BDS_PER_PKT; Pkts++) {
				u32 CrBits = 0;

				Value = TEST_START_VALUE;
				TxPacket = (u8 *)XMcdma_BdRead64(BdCurPtr, XMCDMA_BD_BUFA_OFFSET);
				for(Index1 = 0; Index1 < MAX_PKT_LEN; Index1++) {
					TxPacket[Index1] = Value;

					Value = (Value + 1) & 0xFF;
				}

				if (!McDmaInstPtr->Config.IsTxCacheCoherent)
					Xil_DCacheFlushRange((UINTPTR)TxPacket, MAX_PKT_LEN);

				if (Pkts == 0) {
					CrBits |= XMCDMA_BD_CTRL_SOF_MASK;
				}

				if (Pkts == (NUMBER_OF_BDS_PER_PKT - 1)) {
					CrBits |= XMCDMA_BD_CTRL_EOF_MASK;
				}
				XMcDma_BdSetCtrl(BdCurPtr, CrBits);
				XMCDMA_CACHE_FLUSH((UINTPTR)(BdCurPtr));
				BdCurPtr = (XMcdma_Bd *)XMcdma_BdChainNextBd(Tx_Chan, BdCurPtr);
			}
		}

		BdCurPtr = (XMcdma_Bd *)(TX_BD_SPACE_BASE +
				   (sizeof(XMcdma_Bd) * NUMBER_OF_BDS_TO_TRANSFER * ChanId));
		Status = XMcDma_ChanToHw(Tx_Chan);
		if (Status != XST_SUCCESS) {
			xil_printf("XMcDma_ChanToHw failed for Channel %d\n\r", ChanId);
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

static void DoneHandler(void *CallBackRef, u32 Chan_id)
{
        XMcdma *InstancePtr = (XMcdma *)((void *)CallBackRef);
        XMcdma_ChanCtrl *Rx_Chan = 0;
        XMcdma_Bd *BdPtr1, *FreeBdPtr;
        u8 *RxPacket;
        int ProcessedBdCount, i;
        int MaxTransferBytes;
        int RxPacketLength;

        Rx_Chan = XMcdma_GetMcdmaRxChan(InstancePtr, Chan_id);
        ProcessedBdCount = XMcdma_BdChainFromHW(Rx_Chan, NUMBER_OF_BDS_TO_TRANSFER, &BdPtr1);
        RxDone += ProcessedBdCount;

        FreeBdPtr = BdPtr1;
        MaxTransferBytes = MAX_TRANSFER_LEN(InstancePtr->Config.MaxTransferlen - 1);

        for (i = 0; i < ProcessedBdCount; i++) {
		RxPacket = (void *)XMcdma_BdRead64(FreeBdPtr, XMCDMA_BD_BUFA_OFFSET);
		RxPacketLength = XMcDma_BdGetActualLength(FreeBdPtr, MaxTransferBytes);
		/* Invalidate the DestBuffer before receiving the data, in case
		 * the data cache is enabled
		 */
		if (!InstancePtr->Config.IsRxCacheCoherent)
			Xil_DCacheInvalidateRange((UINTPTR)RxPacket, RxPacketLength);

                if (CheckData((void *)RxPacket, RxPacketLength) != XST_SUCCESS) {
                        xil_printf("Data check failed for the Chan %x\n\r", Chan_id);
                }
                FreeBdPtr = (XMcdma_Bd *) XMcdma_BdRead64(FreeBdPtr, XMCDMA_BD_NDESC_OFFSET);
        }

        RxChanDone += 1;
}

static void ErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask)
{
	xil_printf("Inside error Handler Chan_id is %d\n\r", Chan_id);
	Error = 1;
}

static void TxDoneHandler(void *CallBackRef, u32 Chan_id)
{
	XMcdma *InstancePtr = (XMcdma *)((void *)CallBackRef);
	XMcdma_ChanCtrl *Tx_Chan = 0;
	XMcdma_Bd *BdPtr1;
	int ProcessedBdCount;

	Tx_Chan = XMcdma_GetMcdmaTxChan(InstancePtr, Chan_id);
	ProcessedBdCount = XMcdma_BdChainFromHW(Tx_Chan, NUMBER_OF_BDS_TO_TRANSFER, &BdPtr1);

	TxDone += ProcessedBdCount;
	TxChanDone += 1;
}

static void TxErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask)
{
	xil_printf("Inside Tx error Handler Chan_id is %d and Mask %x\n\r", Chan_id, Mask);
	Error = 1;
}



