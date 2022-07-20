/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xinterrupt_wrap.c
* @addtogroup common_v1_2
* @{
*
* Contains wrapper functions for the scugic/axi intc Interrupt controller
* drivers..
******************************************************************************/

#include "xinterrupt_wrap.h"

#if defined (XPAR_SCUGIC) /* available in xscugic.h */
static XScuGic XScuGicInstance;
static int ScuGicInitialized;
#endif

#if defined (AXI_INTC) /* available in xintc.h */
static XIntc XIntcInstance ;
#endif


int XConfigInterruptCntrl(UINTPTR IntcParent) {
	int Status = XST_FAILURE;
	UINTPTR BaseAddr = XGet_BaseAddr(IntcParent);

	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		XScuGic_Config *CfgPtr = NULL;
		if (XScuGicInstance.IsReady != XIL_COMPONENT_IS_READY) {
			CfgPtr = XScuGic_LookupConfig(BaseAddr);
			if (!ScuGicInitialized) {
				Status = XScuGic_CfgInitialize(&XScuGicInstance, CfgPtr, 0);
			}
			else if (XScuGic_IsInitialized(BaseAddr) != 1U) {
				Status = XScuGic_CfgInitialize(&XScuGicInstance, CfgPtr, 0);
			} else {
				Status = XST_SUCCESS;
			}
		} else {
			Status = XST_SUCCESS;
		}
		return Status;
#else
		return XST_FAILURE;
#endif
	} else {
#if defined (AXI_INTC)
		if (XIntcInstance.IsStarted != XIL_COMPONENT_IS_STARTED)
			Status = XIntc_Initialize(&XIntcInstance, BaseAddr);
		else
			Status = XST_SUCCESS;
		return Status;
#else
		return XST_FAILURE;
#endif
	}
}

int XConnectToInterruptCntrl(u32 IntrId, void *IntrHandler, void *CallBackRef, UINTPTR IntcParent) 
{
	int Status;
	int Doconnect = FALSE;
        UINTPTR BaseAddr = XGet_BaseAddr(IntcParent);

	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC)
	{

#if defined (XPAR_SCUGIC)
		if (ScuGicInitialized) {
			Doconnect = 1;
		} else if (XScuGic_IsInitialized(BaseAddr) == 1U) {
			Doconnect = 1;
		}
                if (Doconnect) {
			u16 IntrNum = XGet_IntrId(IntrId);
			u16 Offset = XGet_IntrOffset(IntrId);
		
			IntrNum += Offset;
			Status = XScuGic_Connect(&XScuGicInstance, IntrNum,  \
			(Xil_ExceptionHandler) IntrHandler, CallBackRef);
			return Status;
		} else {
			return XST_SUCCESS;
		}
#else
		return XST_FAILURE;
#endif
	} else {
#if defined (AXI_INTC)
		Status = XIntc_Connect(&XIntcInstance, IntrId, \
			(XInterruptHandler)IntrHandler, CallBackRef); 
		return Status;
#else
		return XST_FAILURE;
#endif

	}
}


int XDisconnectInterruptCntrl(u32 IntrId, UINTPTR IntcParent)
{
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		u16 IntrNum = XGet_IntrId(IntrId);
		u16 Offset = XGet_IntrOffset(IntrId);
		
		IntrNum += Offset;
		XScuGic_Disconnect(&XScuGicInstance, IntrNum);
#else
		return XST_FAILURE;
#endif
	} else {
#if defined (AXI_INTC)
		XIntc_Disconnect(&XIntcInstance, IntrId);
#else
		return XST_FAILURE;
#endif
	}
	return XST_SUCCESS;
}

int XStartInterruptCntrl(u32 Mode, UINTPTR IntcParent)
{
	int Status = XST_FAILURE;

	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC)
	{
		/* 
		 * For XPAR_SCUGIC, XConfigInterruptCntrl starts controller 
		 * hence returning without doing anything
		 */
		return 0;
	} else  {
#if defined (AXI_INTC)
		if (XIntcInstance.IsStarted != XIL_COMPONENT_IS_STARTED)
			Status = XIntc_Start(&XIntcInstance, Mode);
		else
			Status = XST_SUCCESS;
		return Status;
#else
		return XST_FAILURE;
#endif
	
	}
}


void XEnableIntrId( u32 IntrId, UINTPTR IntcParent)
{
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC)
	{
#if defined (XPAR_SCUGIC)
		u16 IntrNum = XGet_IntrId(IntrId);
		u16 Offset = XGet_IntrOffset(IntrId);
		IntrNum += Offset;
		XScuGic_Enable(&XScuGicInstance, IntrNum);
#endif
		
	} else {
#if defined (AXI_INTC)
		XIntc_Enable(&XIntcInstance, IntrId);
#endif	
	}
}

void XDisableIntrId( u32 IntrId, UINTPTR IntcParent)
{
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		u16 IntrNum = XGet_IntrId(IntrId);
		u16 Offset = XGet_IntrOffset(IntrId);
		
		IntrNum += Offset;
		XScuGic_Disable(&XScuGicInstance, IntrNum);
#endif
	} else {
#if defined (AXI_INTC)
		XIntc_Disable(&XIntcInstance, IntrId);
#endif
	}
}

void XSetPriorityTriggerType( u32 IntrId, u8 Priority, UINTPTR IntcParent)
{
	u8 Trigger = (((XGet_TriggerType(IntrId) == 1) || (XGet_TriggerType(IntrId) == 2)) ? XINTR_IS_EDGE_TRIGGERED
                                                                                : XINTR_IS_LEVEL_TRIGGERED);
	u16 IntrNum = XGet_IntrId(IntrId);
        u16 Offset = XGet_IntrOffset(IntrId);
                                
        IntrNum += Offset;
        if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
        	XScuGic_SetPriorityTriggerType(&XScuGicInstance, IntrNum, Priority, Trigger);
#endif
        }
}

void XGetPriorityTriggerType( u32 IntrId, u8 *Priority, u8 *Trigger,  UINTPTR IntcParent)
{
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		XScuGic_GetPriorityTriggerType(&XScuGicInstance, IntrId, Priority, Trigger);
#endif
	}
}

void XStopInterruptCntrl( UINTPTR IntcParent)
{
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		XScuGic_Stop(&XScuGicInstance);
#endif
	} else {
#if defined (AXI_INTC)
	XIntc_Stop(&XIntcInstance);
#endif
	}
}

void XRegisterInterruptHandler(void *IntrHandler,  UINTPTR IntcParent)
{
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		if (IntrHandler == NULL) {
			Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, \
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			&XScuGicInstance);
		} else {
			Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, \
				(Xil_ExceptionHandler) IntrHandler,
				&XScuGicInstance);

		}
#endif
	} else {
#if defined (AXI_INTC)
		if (IntrHandler == NULL) {
			Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, \
				(Xil_ExceptionHandler) XIntc_InterruptHandler,
				&XIntcInstance);
		} else {
			Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, \
				(Xil_ExceptionHandler) IntrHandler,
				&XIntcInstance);
		}
#endif
	}
}


int XSetupInterruptSystem(void *DriverInstance, void *IntrHandler, u32
IntrId,  UINTPTR IntcParent, u16 Priority)
{
	int Status;

	Status = XConfigInterruptCntrl(IntcParent);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
	XSetPriorityTriggerType( IntrId, Priority, IntcParent);
	Status = XConnectToInterruptCntrl( IntrId, (Xil_ExceptionHandler) IntrHandler, \
				DriverInstance, IntcParent);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
#if defined (AXI_INTC)
	XStartInterruptCntrl(XIN_REAL_MODE, IntcParent);
#endif
	XRegisterInterruptHandler(NULL, IntcParent);
	XEnableIntrId(IntrId, IntcParent);
	Xil_ExceptionInit();
	Xil_ExceptionEnable();
#if defined (XPAR_SCUGIC)
	ScuGicInitialized = TRUE;
#endif
	return XST_SUCCESS;
}
