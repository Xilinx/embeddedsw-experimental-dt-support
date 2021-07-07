/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_debug.c
*
* This is the file which contains uart initialization code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/13/2018 Initial release
* 1.01  ma   08/01/2019 Added LPD init code
* 1.02  ana  11/26/2019 Updated Uart Device ID
*       kc   01/16/2020 Removed xilpm dependency in PLMI for UART
*       ma   02/18/2020 Added support for logging terminal prints
*       ma   03/02/2020 Implement PLMI own outbyte to support logging as well
*       bsv  04/04/2020 Code clean up
* 1.03  kc   07/28/2020 Moved LpdInitialized from xplmi_debug.c to xplmi.c
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
* 1.04  bm   02/01/2021 Add XPlmi_Print functions using xil_vprintf
*       ma   03/24/2021 Store DebugLog structure to RTCA
*       ma   03/24/2021 Print logs to memory when PrintToBuf is TRUE
*       td   05/20/2021 Support user configurable uart baudrate
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi.h"
#include "xplmi_debug.h"
#ifdef DEBUG_UART_PS
#include "xuartpsv.h"
#endif
#include "xil_types.h"
#include "xstatus.h"
#include "xplmi_hw.h"
#include "xplmi_status.h"
#include "xparameters.h"
#include <stdarg.h>

/* PLM specific outbyte function */
void outbyte(char8 c);

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/* SPP Input Clk Freq should be 25 MHz */
#define XPLMI_SPP_INPUT_CLK_FREQ	(25000000U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function initializes the PS UART
 *
 * @param	None
 *
 * @return	Returns XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_InitUart(void)
{
	int Status = XST_FAILURE;

	/* Initialize UART */
	/* If UART is already initialized, just return success */
	if ((LpdInitialized & UART_INITIALIZED) == UART_INITIALIZED) {
		Status = XST_SUCCESS;
		goto END;
	}

#if (XPAR_XUARTPSV_NUM_INSTANCES > 0U)
	u8 Index = 0U;
	XUartPsv UartPsvIns;
	XUartPsv_Config *Config;

#ifndef SDT
	for (Index = 0U; Index < (u8)XPAR_XUARTPSV_NUM_INSTANCES; Index++) {
#else
	for (Index = 0U; XUartPsv_ConfigTable[Index].Name != NULL; Index++) {

#endif
		Status = XPlmi_MemSetBytes(&UartPsvIns, sizeof(XUartPsv),
				0U, sizeof(XUartPsv));
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_UART_MEMSET, Status);
			goto END;
		}

#ifndef SDT
		Config = XUartPsv_LookupConfig(Index);
#else
		Config = XUartPsv_LookupConfig(XUartPsv_ConfigTable[Index].BaseAddress);
#endif
		if (NULL == Config) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_UART_LOOKUP, (int)Index);
			goto END;
		}

		if (XPLMI_PLATFORM == PMC_TAP_VERSION_SPP) {
			Config->InputClockHz = XPLMI_SPP_INPUT_CLK_FREQ;
		}

		Status = XUartPsv_CfgInitialize(&UartPsvIns, Config,
				Config->BaseAddress);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_UART_CFG, Status);
			goto END;
		}
		Status = XUartPsv_SetBaudRate(&UartPsvIns, Config->BaudRate);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_UART_PSV_SET_BAUD_RATE,
					Status);
			goto END;
		}
	}

#ifndef PLM_PRINT_NO_UART
	LpdInitialized |= UART_INITIALIZED;
#endif
#endif

#ifdef DEBUG_UART_MDM
	LpdInitialized |= UART_INITIALIZED;
#endif

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function prints and logs the terminal prints to debug log buffer
 *
 * @param	c is the character to be printed and logged
 *
 * @return	None
 *
 *****************************************************************************/
void outbyte(char8 c)
{
	u64 CurrentAddr;
#ifdef STDOUT_BASEADDRESS
	if(((LpdInitialized) & UART_INITIALIZED) == UART_INITIALIZED) {
		XUartPsv_SendByte(STDOUT_BASEADDRESS, (u8)c);
	}
#endif

	if (DebugLog->PrintToBuf == (u8)TRUE) {
		CurrentAddr = DebugLog->LogBuffer.StartAddr + DebugLog->LogBuffer.Offset;
		if (CurrentAddr >=
				(DebugLog->LogBuffer.StartAddr + DebugLog->LogBuffer.Len)) {
			DebugLog->LogBuffer.Offset = 0x0U;
			DebugLog->LogBuffer.IsBufferFull = TRUE;
			CurrentAddr = DebugLog->LogBuffer.StartAddr;
		}

		XPlmi_OutByte64(CurrentAddr, (u8)c);
		++DebugLog->LogBuffer.Offset;
	}
}

/*****************************************************************************/
/**
 * @brief   This function prints debug messages with timestamp
 *
 * @param   DebugType is the PLM Debug level for the message
 * @param   Ctrl1 is the format specified string to print
 * @param   ... variable arguments that define the format specifiers in Ctrl1
 *
 *****************************************************************************/
void XPlmi_Print(u32 DebugType, const char8 *Ctrl1, ...)
{
	va_list Args;

	va_start(Args, Ctrl1);

	if(((DebugType) & (DebugLog->LogLevel)) != (u8)FALSE) {
		XPlmi_PrintPlmTimeStamp();
		xil_vprintf(Ctrl1, Args);
	}
	va_end(Args);
}

/*****************************************************************************/
/**
 * @brief   This function prints debug messages without timestamp
 *
 * @param   DebugType is the PLM Debug level for the message
 * @param   Ctrl1 is the format specified string to print
 * @param   ... variable arguments that define the format specifiers in Ctrl1
 *
 *****************************************************************************/
void XPlmi_Print_WoTS(u32 DebugType, const char8 *Ctrl1, ...)
{
	va_list Args;

	va_start(Args, Ctrl1);

	if(((DebugType) & (DebugLog->LogLevel)) != (u8)FALSE) {
		xil_vprintf(Ctrl1, Args);
	}
	va_end(Args);
}
