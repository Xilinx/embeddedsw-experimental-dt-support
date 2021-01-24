/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmon_sinit.c
* @addtogroup sysmon_v7_7
* @{
*
* This file contains the implementation of the XSysMon driver's static
* initialization functionality.
*
* @note	None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a xd/sv  05/22/07 First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsysmon.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XSysMon_Config XSysMon_ConfigTable[];

/*****************************************************************************/
/**
*
* This function looks up the device configuration based on the unique device ID.
* The table XSysMon_ConfigTable contains the configuration info for each device
* in the system.
*
* @param	DeviceId contains the ID of the device for which the
*		device configuration pointer is to be returned.
*
* @return
*		- A pointer to the configuration found.
*		- NULL if the specified device ID was not found.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XSysMon_Config *XSysMon_LookupConfig(u16 DeviceId)
{
	XSysMon_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0; Index < XPAR_XSYSMON_NUM_INSTANCES; Index++) {
		if (XSysMon_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XSysMon_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XSysMon_Config *XSysMon_LookupConfig(u32 BaseAddress)
{
	XSysMon_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0; XSysMon_ConfigTable[Index].Name != NULL; Index++) {
		if ((XSysMon_ConfigTable[Index].BaseAddress == BaseAddress)  ||
			!BaseAddress) {
			CfgPtr = &XSysMon_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
