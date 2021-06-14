/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

#ifndef XINTERRUPT_WRAP_H		/* prevent circular inclusions */
#define XINTERRUPT_WRAP_H		/* by using protection macros */

#include "xcommon_drv_config.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_exception.h"

#if defined(AXI_INTC)
#include "xintc.h"
#endif

#if defined(XPAR_SCUGIC)
#include "xscugic.h"
#define XSPI_INTR_OFFSET		32U
#endif

#define XINTERRUPT_DEFAULT_PRIORITY     0xA0U /* AXI INTC doesnt support priority setting, it is default priority for GIC interrupts */
#define XINTC_TYPE_IS_SCUGIC		0U
#define XINTC_TYPE_IS_INTC		1U
#define XINTR_IS_EDGE_TRIGGERED		3U
#define XINTR_IS_LEVEL_TRIGGERED	1U

#define XINTC_TYPE_MASK		0x1
#define XINTC_INTR_TYPE_MASK		0x100000
#define XINTC_BASEADDR_MASK		0xFFFFFFFFFFFFFFFE
#define XINTC_INTRID_MASK		0xFFF
#define XINTC_TRIGGER_MASK		0xF000
#define XINTC_TRIGGER_SHIFT		12
#define XINTC_INTR_TYPE_SHIFT		20U
#define XGet_IntcType(IntParent)	(IntParent & XINTC_TYPE_MASK)
#define XGet_IntrType(IntId)		((IntId & XINTC_INTR_TYPE_MASK) >> XINTC_INTR_TYPE_SHIFT) 
#define XGet_BaseAddr(IntParent)	(IntParent & XINTC_BASEADDR_MASK)
#define XGet_IntrId(IntId)		(IntId & XINTC_INTRID_MASK)
#define XGet_TriggerType(IntId)		((IntId & XINTC_TRIGGER_MASK) >> XINTC_TRIGGER_SHIFT)
#define XGet_IntrOffset(IntId)		(( XGet_IntrType(IntId) == 1) ? 16 : 32) /* For PPI offset is 16 and for SPI it is 32 */

extern int XConfigInterruptCntrl(UINTPTR IntcParent);
extern int XConnectToInterruptCntrl(u32 IntrId, void *IntrHandler, void *CallBackRef, UINTPTR IntcParent);
extern int XDisconnectInterruptCntrl(u32 IntrId, UINTPTR IntcParent);
extern int XStartInterruptCntrl(u32 Mode, UINTPTR IntcParent);
extern void XEnableIntrId( u32 IntrId, UINTPTR IntcParent);
extern void XDisableIntrId( u32 IntrId, UINTPTR IntcParent);
extern void XSetPriorityTriggerType( u32 IntrId, u8 Priority, UINTPTR IntcParent);
extern void XGetPriorityTriggerType( u32 IntrId, u8 *Priority, u8 *Trigger, UINTPTR IntcParent);
extern void XStopInterruptCntrl( UINTPTR IntcParent);
extern void XRegisterInterruptHandler( void *IntrHandler, UINTPTR IntcParent);
extern int XSetupInterruptSystem(void *DriverInstance, void *IntrHandler, u32 IntrId,  UINTPTR IntcParent, u16 Priority);

#endif  /* end of protection macro */
