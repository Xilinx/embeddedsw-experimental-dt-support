/******************************************************************************
* Copyright (C) 2016 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_sinit.c
* @addtogroup sysmonpsv_v2_2
*
* Functions in this file are the minimum required functions for the XSysMonPsv
* driver. See xsysmonpsv.h for a detailed description of the driver.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date	    Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   aad    20/11/18 First release.
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xsysmonpsv_hw.h"
#include "xsysmonpsv.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
extern XSysMonPsv_Config XSysMonPsv_ConfigTable[];

/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the unique device
* ID. The table XSysmonPsu_ConfigTable[] contains the configuration information
* for each device in the system.
*
* @param	None.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device , or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XSysMonPsv_Config *XSysMonPsv_LookupConfig(void)
{
	XSysMonPsv_Config *CfgPtr = NULL;
	u32 Index;

#ifndef SDT
	for (Index = 0U; Index < (u32)XPAR_XSYSMONPSV_NUM_INSTANCES; Index++) {
			CfgPtr = &XSysMonPsv_ConfigTable[Index];
	}
#else
	CfgPtr = &XSysmonpsv_ConfigTable[0];
#endif

	return CfgPtr;
}
