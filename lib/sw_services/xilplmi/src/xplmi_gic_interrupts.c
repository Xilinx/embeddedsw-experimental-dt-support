/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_gic_interrupts.c
*
* This file is to handle the GIC interrupts
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  ma   10/08/2018 Initial release
* 1.01  kc   04/09/2019 Added code to register/enable/disable interrupts
* 1.02  bsv  04/04/2020 Code clean up
* 1.03  bm   10/14/2020 Code clean up
* 1.04  bm   04/03/2021 Move task creation out of interrupt context
* 1.05  ma   07/12/2021 Minor updates to task related code
*       bsv  08/02/2021 Code clean up to reduce code size
*       ma   08/05/2021 Add separate task for each IPI channel
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_gic_interrupts.h"
#include "xplmi_hw.h"
#include "xplmi_util.h"
#include "xplmi_debug.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlmi_GicIntrAddTask(u32 Index);
static void XPlmi_GicAddTask(u32 PlmIntrId);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This will register the GIC handler.
 *
 * @param	GicPVal indicates GICP source
 * @param	GicPxVal indicates GICPx source
 * @param	Handler is the interrupt handler
 * @param	Data is the pointer to arguments to interrupt handler
 *
 * @return	None
 *
 *****************************************************************************/
int XPlmi_GicRegisterHandler(u32 GicPVal, u32 GicPxVal, GicIntHandler_t Handler,
	void *Data)
{
	int Status = XST_FAILURE;
	XPlmi_TaskNode *Task = NULL;
	u32 PlmIntrId = (GicPVal << 8U) | (GicPxVal << 16U);

	Task = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_0, Handler, Data);
	if (Task == NULL) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_TASK_CREATE, 0);
		XPlmi_Printf(DEBUG_GENERAL, "GIC Interrupt task creation "
			"error\n\r");
		goto END;
	}
	Task->IntrId = PlmIntrId | XPLMI_IOMODULE_PMC_GIC_IRQ;
	Task->State |= (u8)XPLMI_TASK_IS_PERSISTENT;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This will clear the GIC interrupt.
 *
 * @param	GicPVal indicates GICP source
 * @param	GicPxVal indicates GICPx source
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicIntrClearStatus(u32 GicPVal, u32 GicPxVal)
{
	u32 GicPMask;
	u32 GicPxMask;

	/* Get the GicP mask */
	GicPMask = (u32)1U << GicPVal;
	GicPxMask = (u32)1U << GicPxVal;
	/* Clear interrupt */
	XPlmi_UtilRMW(PMC_GLOBAL_GICP_PMC_IRQ_STATUS, GicPMask, GicPMask);
	XPlmi_UtilRMW(PMC_GLOBAL_GICP0_IRQ_STATUS + (GicPVal * XPLMI_GICPX_LEN),
		GicPxMask, GicPxMask);
}

/*****************************************************************************/
/**
 * @brief	This will enable the GIC interrupt.
 *
 * @param	GicPVal indicates GICP source
 * @param	GicPxVal indicates GICPx source
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicIntrEnable(u32 GicPVal, u32 GicPxVal)
{
	u32 GicPMask;
	u32 GicPxMask;

	/* Get the GicP mask */
	GicPMask = (u32)1U << GicPVal;
	GicPxMask = (u32)1U << GicPxVal;

	/* Enable interrupt */
	XPlmi_UtilRMW(PMC_GLOBAL_GICP_PMC_IRQ_ENABLE, GicPMask, GicPMask);
	XPlmi_UtilRMW(PMC_GLOBAL_GICP0_IRQ_ENABLE + (GicPVal * XPLMI_GICPX_LEN),
		GicPxMask, GicPxMask);
}

/*****************************************************************************/
/**
 * @brief	This will disable the GIC interrupt.
 *
 * @param	GicPVal indicates GICP source
 * @param	GicPxVal indicates GICPx source
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicIntrDisable(u32 GicPVal, u32 GicPxVal)
{
	u32 GicPxMask;

	/* Get the GicP mask */
	GicPxMask = (u32)1U << GicPxVal;
	/* Disable interrupt */
	XPlmi_UtilRMW(PMC_GLOBAL_GICP0_IRQ_DISABLE + (GicPVal * XPLMI_GICPX_LEN),
		GicPxMask, GicPxMask);
}

/*****************************************************************************/
/**
 * @brief	This is handler for GIC interrupts.
 *
 * @param	CallbackRef is presently the interrupt number that is received
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GicIntrHandler(void *CallbackRef)
{
	u32 GicPIntrStatus;
	u32 GicPNIntrStatus;
	u32 GicPNIntrMask;
	u32 GicIndex;
	u32 GicPIndex;
	u32 GicOffset;
	u32 GicPMask;
	u32 GicMask;
	u32 PlmIntrId;

	(void)CallbackRef;

	GicPIntrStatus = XPlmi_In32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS);
	XPlmi_Printf(DEBUG_DETAILED,
			"GicPIntrStatus: 0x%x\r\n", GicPIntrStatus);

	for (GicIndex = 0U; GicIndex < XPLMI_GICP_SOURCE_COUNT; GicIndex++) {
		GicMask = (u32)1U << GicIndex;
		if ((GicPIntrStatus & GicMask) == 0U) {
			continue;
		}
		GicOffset = GicIndex * XPLMI_GICPX_LEN;
		GicPNIntrStatus = XPlmi_In32(PMC_GLOBAL_GICP0_IRQ_STATUS +
			GicOffset);
		GicPNIntrMask = XPlmi_In32(PMC_GLOBAL_GICP0_IRQ_MASK +
			GicOffset);
		XPlmi_Printf(DEBUG_DETAILED, "GicP%u Intr Status: 0x%x\r\n",
			GicIndex, GicPNIntrStatus);

		for (GicPIndex = 0U; GicPIndex < XPLMI_NO_OF_BITS_IN_REG;
			GicPIndex++) {
			GicPMask = (u32)1U << GicPIndex;
			if ((GicPNIntrStatus & GicPMask) == 0U) {
				continue;
			}
			if ((GicPNIntrMask & GicPMask) != 0U) {
				continue;
			}
			PlmIntrId = ((GicIndex << XPLMI_GICP_INDEX_SHIFT) |
					(GicPIndex << XPLMI_GICPX_INDEX_SHIFT));
			XPlmi_GicAddTask(PlmIntrId);
			XPlmi_Out32((PMC_GLOBAL_GICP0_IRQ_DISABLE + GicOffset), GicPMask);
			XPlmi_Out32(PMC_GLOBAL_GICP0_IRQ_STATUS + GicOffset, GicPMask);
		}
		XPlmi_Out32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS, GicMask);
	}

	return;
}

/*****************************************************************************/
/**
 * @brief	This will add the GIC interrupt task handler to the TaskQueue.
 *
 * @param	PlmIntrId is the GIC interrupt ID of the task
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_GicAddTask(u32 PlmIntrId)
{
#if defined(XPAR_XIPIPSU_0_DEVICE_ID) || defined(XPAR_XIPIPSU_0_BASEADDR)
	u16 IpiIntrVal;
	u16 IpiMaskVal;
	u8 IpiIndex;
	u16 IpiIndexMask;

	/* Check if the received interrupt is IPI */
	if (PlmIntrId == XPLMI_IPI_INTR_ID) {
		IpiIntrVal = (u16)Xil_In32(IPI_PMC_ISR);
		IpiMaskVal = (u16)Xil_In32(IPI_PMC_IMR);
		/*
		 * Check IPI source channel and add channel specific task to
		 * task queue according to the channel priority
		 */
		for (IpiIndex = 0U; IpiIndex < XPLMI_IPI_MASK_COUNT; ++IpiIndex) {
			IpiIndexMask = (u16)1U << IpiIndex;
			if (((IpiIntrVal & IpiIndexMask) != 0U) &&
				((IpiMaskVal & IpiIndexMask) == 0U)) {
				XPlmi_GicIntrAddTask(PlmIntrId |
					((u32)IpiIndex << XPLMI_IPI_INDEX_SHIFT));
			}
		}
	} else
#endif
	{
		/* Add task to the task queue */
		XPlmi_GicIntrAddTask(PlmIntrId);
	}
}

/*****************************************************************************/
/**
 * @brief	This function adds the GiC task handler to the TaskQueue.
 *
 * @param	Index is the interrupt index with GicPx and its corresponding
 *              bit details.
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_GicIntrAddTask(u32 Index)
{
	XPlmi_TaskNode *Task = NULL;
	u32 IntrId = Index | XPLMI_IOMODULE_PMC_GIC_IRQ;

	Task = XPlmi_GetTaskInstance(NULL, NULL, IntrId);
	if (Task == NULL) {
		XPlmi_Printf(DEBUG_GENERAL, "GIC Interrrupt add task error\n\r");
		goto END;
	}
	XPlmi_TaskTriggerNow(Task);

END:
	return;
}
