/******************************************************************************
* Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xqspipsu_sinit.c
* @addtogroup qspipsu_v1_13
* @{
*
* The implementation of the XQspiPsu component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   hk  08/21/14 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xqspipsu.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#ifndef SDT
extern XQspiPsu_Config XQspiPsu_ConfigTable[XPAR_XQSPIPSU_NUM_INSTANCES];
#else
extern XQspiPsu_Config XQspiPsu_ConfigTable[];
#endif

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device to look up the
*		configuration for.
*
* @return
*
* A pointer to the configuration found or NULL if the specified device ID was
* not found. See xqspipsu.h for the definition of XQspiPsu_Config.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XQspiPsu_Config *XQspiPsu_LookupConfig(u16 DeviceId)
{
	XQspiPsu_Config *CfgPtr = NULL;
	s32 Index;

	for (Index = 0; Index < XPAR_XQSPIPSU_NUM_INSTANCES; Index++) {
		if (XQspiPsu_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XQspiPsu_ConfigTable[Index];
			break;
		}
	}
	return (XQspiPsu_Config *)CfgPtr;
}
#else
XQspiPsu_Config *XQspiPsu_LookupConfig(u32 BaseAddress)
{
	XQspiPsu_Config *CfgPtr = NULL;
	s32 Index;

	for (Index = 0; XQspiPsu_ConfigTable[Index].Name != NULL; Index++) {
		if ((XQspiPsu_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XQspiPsu_ConfigTable[Index];
			break;
		}
	}
	return (XQspiPsu_Config *)CfgPtr;
}
#endif
/** @} */
