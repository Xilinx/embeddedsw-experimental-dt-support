/*
 * FreeRTOS Kernel V10.4.3
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (C) 2014 - 2020 Xilinx, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 * 1 tab == 4 spaces!
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Xilinx includes. */
#include "xiltimer.h"
#include "bspconfig.h"
#include "xscugic.h"
#include "FreeRTOSConfig.h"

void vApplicationAssert( const char *pcFileName, uint32_t ulLine )
		__attribute__((weak));

extern uintptr_t IntrControllerAddr;

/*-----------------------------------------------------------*/

void TimerCounterHandler(void *CallBackRef, u32 TmrCtrNumber)
{
        FreeRTOS_Tick_Handler();
}

void FreeRTOS_SetupTickInterrupt( void )
{
	/*
	 * The Xilinx implementation of generating run time task stats uses the same timer used for generating
	 * FreeRTOS ticks. In case user decides to generate run time stats the timer time out interval is changed
	 * as "configured tick rate * 10". The multiplying factor of 10 is hard coded for Xilinx FreeRTOS ports.
	 */
#if (configGENERATE_RUN_TIME_STATS == 1)
	/* XTimer_SetInterval() API expects delay in milli seconds
         * Convert the user provided tick rate to milli seconds.
         */
	XTimer_SetInterval((configTICK_RATE_HZ * 10)/10);
#else
	/* XTimer_SetInterval() API expects delay in milli seconds
         * Convert the user provided tick rate to milli seconds.
         */
	XTimer_SetInterval(configTICK_RATE_HZ/10);
#endif
	XTimer_SetHandler(TimerCounterHandler, 0);
	XTimer_SetTickPriority(portLOWEST_USABLE_INTERRUPT_PRIORITY << portPRIORITY_SHIFT);
}
/*-----------------------------------------------------------*/

void vApplicationIRQHandler( uint32_t ulICCIAR )
{
extern const XScuGic_Config XScuGic_ConfigTable[];
static const XScuGic_VectorTableEntry *pxVectorTable = XScuGic_ConfigTable[0].HandlerTable;
uint32_t ulInterruptID;
const XScuGic_VectorTableEntry *pxVectorEntry;

	/* Interrupts cannot be re-enabled until the source of the interrupt is
	cleared. The ID of the interrupt is obtained by bitwise ANDing the ICCIAR
	value with 0x3FF. */
	ulInterruptID = ulICCIAR & 0x3FFUL;

	if( ulInterruptID < XSCUGIC_MAX_NUM_INTR_INPUTS )
	{
		/* Call the function installed in the array of installed handler
		functions. */
		pxVectorEntry = &( pxVectorTable[ ulInterruptID ] );
		configASSERT( pxVectorEntry );
		pxVectorEntry->Handler( pxVectorEntry->CallBackRef );
	}
}
/*-----------------------------------------------------------*/

/* This version of vApplicationAssert() is declared as a weak symbol to allow it
to be overridden by a version implemented within the application that is using
this BSP. */
void vApplicationAssert( const char *pcFileName, uint32_t ulLine )
{
volatile uint32_t ul = 0;
volatile const char *pcLocalFileName = pcFileName; /* To prevent pcFileName being optimized away. */
volatile uint32_t ulLocalLine = ulLine; /* To prevent ulLine being optimized away. */

	/* Prevent compile warnings about the following two variables being set but
	not referenced.  They are intended for viewing in the debugger. */
	( void ) pcLocalFileName;
	( void ) ulLocalLine;

	xil_printf( "Assert failed in file %s, line %lu\r\n", pcLocalFileName, ulLocalLine );

	/* If this function is entered then a call to configASSERT() failed in the
	FreeRTOS code because of a fatal error.  The pcFileName and ulLine
	parameters hold the file name and line number in that file of the assert
	that failed.  Additionally, if using the debugger, the function call stack
	can be viewed to find which line failed its configASSERT() test.  Finally,
	the debugger can be used to set ul to a non-zero value, then step out of
	this function to find where the assert function was entered. */
	taskENTER_CRITICAL();
	{
		while( ul == 0 )
		{
			__asm volatile( "NOP" );
		}
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

/* This default tick hook does nothing and is declared as a weak symbol to allow
the application writer to override this default by providing their own
implementation in the application code. */
void vApplicationTickHook( void )
{
}
/*-----------------------------------------------------------*/

/* This default idle hook does nothing and is declared as a weak symbol to allow
the application writer to override this default by providing their own
implementation in the application code. */
void vApplicationIdleHook( void )
{
}
/*-----------------------------------------------------------*/

/* This default malloc failed hook does nothing and is declared as a weak symbol
to allow the application writer to override this default by providing their own
implementation in the application code. */
void vApplicationMallocFailedHook( void )
{
	xil_printf( "vApplicationMallocFailedHook() called\n" );
}
/*-----------------------------------------------------------*/

/* This default stack overflow hook will stop the application for executing.  It
is declared as a weak symbol to allow the application writer to override this
default by providing their own implementation in the application code. */
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
/* Attempt to prevent the handle and name of the task that overflowed its stack
from being optimised away because they are not used. */
volatile TaskHandle_t xOverflowingTaskHandle = xTask;
volatile char *pcOverflowingTaskName = pcTaskName;

	( void ) xOverflowingTaskHandle;
	( void ) pcOverflowingTaskName;

	xil_printf( "HALT: Task %s overflowed its stack.", pcOverflowingTaskName );
	portDISABLE_INTERRUPTS();
	for( ;; );
}
