/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/****************************************************************************/
/**
 *
 * @file xsysmonpsv_intr_example.c
 *
 * This file contains a design example using the driver functions
 * of the System Monitor driver. The example here shows the
 * driver/device in intr mode to check the on-chip temperature and voltages.
 *
 * @note
 *
 * This examples also assumes that there is a STDIO device in the system.
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- -----  -------- -----------------------------------------------------
 * 1.1   aad    2/7/19   First release
 * 1.2   aad    3/19/20  Fixed the interrupt disable flag
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/
#include "xparameters.h"
#include "xscugic.h"
#include "xstatus.h"
#include "xsysmonpsv.h"
#include "xsysmonpsv_hw.h"
#include "xinterrupt_wrap.h"

/************************** Constant Definitions ****************************/
#define INTR_0 0U
#define INTR_1 1U
#define LOCK_CODE 0xF9E8D7C6
#define SYSMONPSV_TIMEOUT 100000

/************************** Function Prototypes *****************************/

int SysMonPsvIntrExample();
static void SysMonPsv_InterruptHandler(void *CallBackRef);

/************************** Variable Definitions ****************************/

static XSysMonPsv SysMonInst; /* System Monitor driver instance */
static XScuGic IntcInst;      /* Interrupt controller instance */
volatile u32 IntrStatus;
volatile u32 ALARM0, ALARM1, ALARM2, ALARM3, ALARM4, OT_ALARM, NEW_DATA;

/****************************************************************************/
/**
 *
 * Main function that invokes the intr example in this file.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS if the example has completed successfully.
 *		- XST_FAILURE if the example has failed.
 *
 * @note		None.
 *
 *****************************************************************************/
int main(void) {
  int Status;
  IntrStatus = 0;
  u32 Timeout = 0;

  Status = SysMonPsvIntrExample();
  if (Status != XST_SUCCESS) {
    xil_printf("Sysmon Intr Example Test Failed\r\n");
    return XST_FAILURE;
  }

  /* wait for interrupts */
  while (!IntrStatus) {
    Timeout++;
    if (Timeout > SYSMONPSV_TIMEOUT) {
      xil_printf("Failed to get an interrupt\r\n");
      return XST_FAILURE;
    }
  }

  xil_printf("Successfully ran Sysmon Intr Example Test\r\n");

  return XST_SUCCESS;
}

/****************************************************************************/
/**
 *
 * This function runs a test on the System Monitor device using the
 * driver APIs.
 * This function does the following tasks:
 *	- Initiate the System Monitor device driver instance
 *	- Enable OT interrupt
 *	- Setup call back for interrupts
 *	- Read supply configuration.
 *	- Read the latest on-chip temperatures and confiured supplies
 *
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS if the example has completed successfully.
 *		- XST_FAILURE if the example has failed.
 *
 * @note		None
 *
 ****************************************************************************/
int SysMonPsvIntrExample() {
  int Status;
  XSysMonPsv_Config *ConfigPtr;
  XSysMonPsv *SysMonInstPtr = &SysMonInst;
  u32 Mask;

  printf("\r\nEntering the SysMon Intr Example. \r\n");

  /* Initialise the SysMon driver. */
  ConfigPtr = XSysMonPsv_LookupConfig();
  if (ConfigPtr == NULL) {
    return XST_FAILURE;
  }

  XSysMonPsv_CfgInitialize(SysMonInstPtr, ConfigPtr);

  /* Unlock the sysmon register space */
  XSysMonPsv_WriteReg(SysMonInstPtr->Config.BaseAddress + XSYSMONPSV_PCSR_LOCK,
                      LOCK_CODE);

  /* Clear any bits set in the Interrupt Status Register. */
  XSysMonPsv_IntrClear(SysMonInstPtr, 0xFFFFFFFF);

  Status = XSetupInterruptSystem(SysMonInstPtr, &SysMonPsv_InterruptHandler,
				 ConfigPtr->IntrId,
				 ConfigPtr->IntrParent,
				 XINTERRUPT_DEFAULT_PRIORITY);
  if (Status != XST_SUCCESS)
    return Status;

  /* *
   * Setup Interrupt for SysMon
   * - OT_ALARM
   * - NEW_DATA0
   * - ALARM0
   * - ALARM1
   * - ALARM2
   * - ALARM3
   * - ALARM4
   * */
  Mask = XSYSMONPSV_IER0_OT_MASK | XSYSMONPSV_IER0_NEW_DATA0_MASK |
         XSYSMONPSV_IER0_ALARM0_MASK | XSYSMONPSV_IER0_ALARM1_MASK |
         XSYSMONPSV_IER0_ALARM2_MASK | XSYSMONPSV_IER0_ALARM3_MASK |
         XSYSMONPSV_IER0_ALARM4_MASK;
  XSysMonPsv_IntrEnable(SysMonInstPtr, Mask, INTR_0);

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function is the Interrupt Service Routine for the System Monitor device.
 * It will be called by the processor whenever an interrupt is asserted
 * by the device.
 *
 * * User of this code may need to modify the code to meet the needs of the
 * application.
 *
 * @param	CallBackRef is the callback reference passed from the Interrupt
 *		controller driver, which in our case is a pointer to the
 *		driver instance.
 *
 * @return	None.
 *
 * @note		This function is called within interrupt context.
 *
 ******************************************************************************/
static void SysMonPsv_InterruptHandler(void *CallBackRef) {
  XSysMonPsv *SysMonPtr = (XSysMonPsv *)CallBackRef;

  /* Get the interrupt status from the device and check the value. */
  IntrStatus = XSysMonPsv_IntrGetStatus(SysMonPtr);

  /* Clear all bits in Interrupt Status Register. */
  XSysMonPsv_IntrClear(SysMonPtr, IntrStatus);

  if (IntrStatus & XSYSMONPSV_ISR_ALARM0_MASK)
    ALARM0++;

  if (IntrStatus & XSYSMONPSV_ISR_ALARM1_MASK)
    ALARM1++;

  if (IntrStatus & XSYSMONPSV_ISR_ALARM2_MASK)
    ALARM2++;

  if (IntrStatus & XSYSMONPSV_ISR_ALARM3_MASK)
    ALARM3++;

  if (IntrStatus & XSYSMONPSV_ISR_ALARM4_MASK)
    ALARM4++;

  if (IntrStatus & XSYSMONPSV_ISR_NEW_DATA0_MASK)
    NEW_DATA++;

  if (IntrStatus & XSYSMONPSV_ISR_OT_MASK)
    OT_ALARM++;

  /* Interrupt disable */
  XSysMonPsv_IntrDisable(SysMonPtr, IntrStatus, INTR_0);
}

