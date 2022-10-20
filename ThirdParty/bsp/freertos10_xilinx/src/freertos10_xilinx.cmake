# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.3)
include(${CMAKE_SOURCE_DIR}/Freertos10_xilinxExample.cmake * NO_POLICY_SCOPE)

find_package(common)
#kernel behavior settings
set(max_api_call_interrupt_priority 18 CACHE STRING "The maximum interrupt \
priority from which interrupt safe FreeRTOS API calls can be made.")
option(use_preemption "The maximum interrupt priority from which interrupt \
safe FreeRTOS API calls can be made." ON)
set(tick_rate 100 CACHE STRING "Number of RTOS ticks per sec")
option(idle_yield "Set to true if the Idle task should yield if another \
idle priority task is able to run, or false if the idle task should \
always use its entire time slice unless it is preempted." ON)
set(max_priorities 8 CACHE STRING "The number of task priorities that \
will be available.  Priorities can be assigned from \
zero to (max_priorities - 1")
set(minimal_stack_size 200 CACHE STRING "The size of the stack allocated \
to the Idle task. Also used by standard demo and test tasks found in \
the main FreeRTOS download.")
set(total_heap_size 65536 CACHE STRING "Sets the amount of RAM reserved \
for use by FreeRTOS - used when tasks, queues, semaphores and \
event groups are created.")
set(max_task_name 10 CACHE STRING "The maximum number of characters \
that can be in the name of a task.")
option(use_timeslicing "When true equal priority ready tasks will share \
CPU time with a context switch on each tick interrupt." ON)
option(use_port_optimized_task_selection "When true task selection will \
be faster at the cost of limiting the maximum number \
of unique priorities to 32." ON)


#kernel feature settings
option(stream_buffer "Set to true to include stream buffer functionality, \
or false to exclude stream buffer functionality." OFF)
option(message_buffer "Set to true to include message buffer functionality, \
or false to exclude message buffer functionality." OFF)
option(support_static_allocation "Set to true to allocate memory statically, \
or false to allocate memory dynamically." OFF)
option(use_freertos_asserts "Defines configASSERT() to assist \
development and debugging.  The application can override the \
default implementation of \
vApplicationAssert( char *pcFile, uint32_t ulLine )" ON)
option(use_mutexes "Set to true to include mutex functionality, \
or false to exclude mutex functionality." ON)
option(use_getmutex_holder "Set to true to use mutex \
xSemaphoreGetMutexHolder API, or false to exclude it." ON)
option(use_recursive_mutexes "Set to true to include recursive mutex \
functionality, or false to exclude recursive mutex functionality." ON)
option(use_counting_semaphores "Set to true to include counting semaphore \
functionality, or false to exclude recursive mutex functionality." ON)
set(queue_registry_size 10 CACHE STRING "The maximum number of queues \
that can be registered at any one time. Only registered queues can be viewed \
in the Eclipse/GDB kernel aware debugger plug-in.")
option(use_trace_facility "Set to true to include the legacy trace \
functionality, and a few other features.  \
traceMACROS are the preferred method of tracing now." ON)
option(use_newlib_reent "When true each task will have its own Newlib \
reent structure." OFF)
option(use_queue_sets "Set to true to include queue set functionality." ON)
option(use_task_notifications "Set to true to include direct to \
task notification functionality." ON)
set(check_for_stack_overflow 2 CACHE STRING "Set to 0 for no overflow checking. \ 
Set to 1 to include basic run time task stack checking.  \
Set to 2 to include more comprehensive run time task stack checking.")
set_property(CACHE check_for_stack_overflow PROPERTY STRINGS 0x0 0x1 0x2)
option(use_stats_formatting_functions "Set to 1 to include the vTaskList() \
and vTaskGetRunTimeStats() functions, which format run-time data \
into human readable text." ON)
set(num_thread_local_storage_pointers 0x0 CACHE STRING "Sets the number \
of pointers each task has to store thread local values.")
set(use_task_fpu_support 0x1 CACHE STRING "Set to 1 to create tasks \
without FPU context, set to 2 to have tasks with FPU context by default.")
set_property(CACHE use_task_fpu_support PROPERTY STRINGS 0x0 0x1 0x2)
set(generate_runtime_stats 0x0 CACHE STRING "Set to 1 generate \
runtime stats for tasks.")
set_property(CACHE generate_runtime_stats PROPERTY STRINGS 0x0 0x1)

#hook function settings
option(use_idle_hook "Set to true for the kernel to call \
vApplicationIdleHook() on each iteration of the idle task.  \
The application must provide an implementation of vApplicationIdleHook()." OFF)
option(use_tick_hook "Set to true for the kernel to call \
vApplicationTickHook() during each tick interrupt.  
The application must provide an implementation of vApplicationTickHook()." OFF)
option(use_malloc_failed_hook "Only used if a FreeRTOS memory manager \
(heap_n.c) is included in the project.  Set to true for the kernel to call \
vApplicationMallocFailedHookHook() if there is insufficient FreeRTOS \
heap available for a task, queue or semaphore to be created.  \
The application can override the default implementation of \
vApplicationMallocFailedHook()." ON)
option(use_daemon_task_startup_hook "Set true for kernel to call \
vApplicationDaemonTaskStartupHook on first iteration of RTOS daemon task. \
The application must provide an implementation of 
vApplicationDaemonTaskStartupHook()." OFF)

#software timer settings
option(use_timers "Set to true to include software timer functionality, \
or false to exclude software timer functionality" ON)
set(timer_task_prio_val configMAX_PRIORITIES-1)
set(timer_task_priority ${timer_task_prio_val} CACHE STRING "The priority \
at which the software timer service/daemon task will execute.")
set(timer_command_queue_length 10 CACHE STRING "The number of commands \
the timer command queue can hold at any one time.")
set(timer_task_stack_depth_val configMINIMAL_STACK_SIZE)
set(timer_task_stack_depth ${timer_task_stack_depth_val} CACHE STRING "The \
size of the stack allocated to the timer service/daemon task.")


#tick setup settings
set(timer_select psu_ttc_0 CACHE STRING "Applicable only for R5. \
Selects the ttc module from which a counter will be used as the \
freertos tick source.")
set_property(CACHE timer_select PROPERTY STRINGS 
            ${TTCPS_NUM_DRIVER_INSTANCES})
set(timer_select_counter 0x0 CACHE STRING "Applicable only for R5. \
Selects the ttc counter number inside the selected ttc module \
to be used as the freertos tick source.")
set_property(CACHE timer_select_counter PROPERTY STRINGS 0x0 0x1 0x2 0x3)

#enable stm trace event settings
#option(enable_timer_tick_trace "Enable tracing of timer tick events" OFF)
#set(stm_channel 0x0 CACHE STRING "STM channel to use for trace. Valid channels are 0-65535.")


set(configMAX_API_CALL_INTERRUPT_PRIORITY ${max_api_call_interrupt_priority})
set(configTICK_RATE_HZ ${tick_rate})
set(configMAX_PRIORITIES ${max_priorities})
math(EXPR minimal_stack_size "((${minimal_stack_size} + 3) & 4294967292)")
set(configMINIMAL_STACK_SIZE ${minimal_stack_size})
set(configTOTAL_HEAP_SIZE ${total_heap_size})
set(configMAX_TASK_NAME_LEN ${max_task_name})
set(configQUEUE_REGISTRY_SIZE ${queue_registry_size})
set(configCHECK_FOR_STACK_OVERFLOW ${check_for_stack_overflow})
set(configNUM_THREAD_LOCAL_STORAGE_POINTERS 
                          ${num_thread_local_storage_pointers})
set(configUSE_TASK_FPU_SUPPORT ${use_task_fpu_support})
set(configTIMER_TASK_PRIORITY ${timer_task_priority})
set(configGENERATE_RUN_TIME_STATS ${generate_runtime_stats})
if("${generate_runtime_stats}" EQUAL 1)
    if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze")
        set(PRINT_GEN_STATS_LINE1 "#ifndef __ASSEMBLER__\n")
        set(PRINT_GEN_STATS_LINE2 
            "void xCONFIGURE_TIMER_FOR_RUN_TIME_STATS(void);\n")
        set(PRINT_GEN_STATS_LINE3 "#endif\n")
        string(CONCAT PRINT_GEN_STATS_LINES "${PRINT_GEN_STATS_LINE1}" 
                                            "${PRINT_GEN_STATS_LINE2}" 
                                            "${PRINT_GEN_STATS_LINE3}")
    else()
        set(PRINT_GEN_STATS_LINES 
            "void xCONFIGURE_TIMER_FOR_RUN_TIME_STATS(void);\n")
    endif()
    set(portCONFIGURE_TIMER_FOR_RUN_TIME_STATS 
    "#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() \ 
              xCONFIGURE_TIMER_FOR_RUN_TIME_STATS()")
    if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze")
        set(PRINT_GET_CNTR_LINE1 "#ifndef __ASSEMBLER__\n")
        set(PRINT_GET_CNTR_LINE2 "uint32_t xGET_RUN_TIME_COUNTER_VALUE(void);\n")
        set(PRINT_GET_CNTR_LINE3 "#endif\n")
        string(CONCAT PRINT_GET_CNTR_LINES "${PRINT_GET_CNTR_LINE1}" 
                                           "${PRINT_GET_CNTR_LINE2}" 
                                           "${PRINT_GET_CNTR_LINE3}")
    else()
        set(PRINT_GET_CNTR_LINES "uint32_t xGET_RUN_TIME_COUNTER_VALUE(void);\n")
    endif()
    set(portGET_RUN_TIME_COUNTER_VALUE "#define portGET_RUN_TIME_COUNTER_VALUE() \
                                        xGET_RUN_TIME_COUNTER_VALUE()")
	
else()
    set(PRINT_GEN_STATS_LINES "")
    set(portCONFIGURE_TIMER_FOR_RUN_TIME_STATS 
                "#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()")
    set(PRINT_GET_CNTR_LINES "")
    set(portGET_RUN_TIME_COUNTER_VALUE 
                "#define portGET_RUN_TIME_COUNTER_VALUE()")
endif()
		
set(configTIMER_QUEUE_LENGTH ${timer_command_queue_length})
set(configTIMER_TASK_STACK_DEPTH ${timer_task_stack_depth})
#set(FREERTOS_STM_CHAN ${stm_channel})
set(configUNIQUE_INTERRUPT_PRIORITIES "")
if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53") OR 
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR 
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa9") OR 
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5") 
   )
    set(configUNIQUE_INTERRUPT_PRIORITIES 32)
endif()
set(configUSE_16_BIT_TICKS 0x0)
set(configUSE_APPLICATION_TASK_TAG 0x0)
set(configUSE_CO_ROUTINES 0x0)
set(configMAX_CO_ROUTINE_PRIORITIES 2)
set(configUSE_TICKLESS_IDLE 0x0)
set(configTASK_RETURN_ADDRESS	NULL)
set(INCLUDE_vTaskPrioritySet 1)
set(INCLUDE_uxTaskPriorityGet 1)
set(INCLUDE_vTaskDelete 1)
set(INCLUDE_vTaskCleanUpResources 1)
set(INCLUDE_vTaskSuspend 1)
set(INCLUDE_vTaskDelayUntil 1)
set(INCLUDE_vTaskDelay 1)
set(INCLUDE_eTaskGetState 1)
set(INCLUDE_xTimerPendFunctionCall 1)
set(INCLUDE_pcTaskGetTaskName 1)

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53") OR
    ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72")
  )
 
    set(portPOINTER_SIZE_TYPE uint64_t)
elseif(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5") OR 
       ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa9") OR 
       ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") 
      )
    set(portPOINTER_SIZE_TYPE uint32_t)
endif()

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53") OR 
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR 
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa9") OR 
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") 
   )
    set(portTICK_TYPE_IS_ATOMIC 0x1)
elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5")
    set(portTICK_TYPE_IS_ATOMIC 0x0)
endif()

set(configMESSAGE_BUFFER_LENGTH_TYPE uint32_t)
set(configSTACK_DEPTH_TYPE uint32_t)

if (${use_preemption})
    set(configUSE_PREEMPTION " ")
endif()
if (${idle_yield})
    set(configIDLE_SHOULD_YIELD " ")
endif()
if (${use_timeslicing})
    set(configUSE_TIME_SLICING " ")
endif()
if (${use_port_optimized_task_selection})
    set(configUSE_PORT_OPTIMIZED_TASK_SELECTION " ")
endif()
if (${stream_buffer})
    set(configSTREAM_BUFFER " ")
endif()
if (${message_buffer})
    set(configMESSAGE_BUFFER " ")
endif()
if (${support_static_allocation})
    set(configSUPPORT_STATIC_ALLOCATION " ")
endif()
if (${use_freertos_asserts})
    set(FREERTOS_ASSERTS "#define configASSERT( x ) \
if( ( x ) == 0 ) vApplicationAssert( __FILE__, __LINE__ )")	
endif()
if (${use_mutexes}) 
    set(configUSE_MUTEXES " ")
endif()
if (${use_getmutex_holder}) 
    set(INCLUDE_xSemaphoreGetMutexHolder " ")
endif()
if (${use_recursive_mutexes}) 
    set(configUSE_RECURSIVE_MUTEXES " ")
endif()
if (${use_counting_semaphores}) 
    set(configUSE_COUNTING_SEMAPHORES " ")
endif()
if (${use_trace_facility}) 
    set(configUSE_TRACE_FACILITY " ")
endif()
if (${use_newlib_reent}) 
    set(configUSE_NEWLIB_REENTRANT " ")
endif()
if (${use_queue_sets}) 
    set(configUSE_QUEUE_SETS " ")
endif()
if (${use_task_notifications}) 
    set(configUSE_TASK_NOTIFICATIONS " ")
endif()
if (${use_stats_formatting_functions}) 
    set(configUSE_STATS_FORMATTING_FUNCTION " ")
endif()
if (${use_idle_hook}) 
    set(configUSE_IDLE_HOOK " ")
endif()
if (${use_tick_hook}) 
    set(configUSE_TICK_HOOK " ")
endif()
if (${use_malloc_failed_hook}) 
    set(configUSE_MALLOC_FAILED_HOOK " ")
endif()
if (${use_daemon_task_startup_hook}) 
    set(configUSE_DAEMON_TASK_STARTUP_HOOK " ")
endif()
if (${use_timers}) 
    set(configUSE_TIMERS " ")
endif()
#if (${enable_timer_tick_trace})
#    set(FREERTOS_TIMER_TICK_TRACE " ")
#endif()

list(LENGTH TTCPS_NUM_DRIVER_INSTANCES CONFIG_TTCPS)
if (${CONFIG_TTCPS})
    set(index 0)
    LIST_INDEX(${index} ${timer_select} "${TTCPS_NUM_DRIVER_INSTANCES}")
    list(GET TOTAL_TTCPS_PROP_LIST ${index} reg)
    set(reg1 ${${reg}})
    set(index1 0)
    list(GET reg1 ${index1} reg2)
    set(configTIMER_BASEADDR ${reg2})
    set(configTIMER_SELECT_CNTR ${timer_select_counter})
else()
    message(FATAL_ERROR "A53, R5 or A72 FreeRTOS need a TTC in the system \
    without it cannot work")
	
endif()


if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53") OR 
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR 
 # ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5") OR 
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa9")
  ) 
    set(index2 0)
    list(GET SCUGIC0_PROP_LIST ${index2} reg3)
    set(configINTERRUPT_CONTROLLER_BASE_ADDRESS ${reg3})
    if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
        set(configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET 0x10000)
    elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72")
        set(configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET 0x10000)
   # elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5")
       # set(configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET 0x1000)
    elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa9")
        set(configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET -0xf00)
    endif()
elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5") 
    set(configINTERRUPT_CONTROLLER_BASE_ADDRESS 0xf9000000)
    set(configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET 0x1000)
endif() 

set(SETUP_TICK_FUN "")
set(CONFIG_SETUP_TICK "")
set(CLEAR_TICK_INTR_FUN "")
set(CLEAR_TICK_INTR_DEF "")
set(CMD_INT_MAX_OUTPUT_SIZE "")
set(RECMU_CNTRL_TASK_PRIORITY "")
set(CMD_INT_MAX_OUTPUT_SIZE "")
set(RECMU_CNTRL_TASK_PRIORITY "")
set(FABS_FUN "")
set(SET_INT_MASK_FROM_ISR "") 
set(INSTALL_EXCEPTION_HANDLERS "")

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze")
    set(APPLICATION_ASSERT_FUN_LINE1 "#ifndef __ASSEMBLER__\n")
    set(APPLICATION_ASSERT_FUN_LINE2 
    "void vApplicationAssert( const char *pcFile, uint32_t ulLine );")
    set(APPLICATION_ASSERT_FUN_LINE1 "#endif\n")
    string(CONCAT APPLICATION_ASSERT_FUN "${APPLICATION_ASSERT_FUN_LINE1}" 
                                         "${APPLICATION_ASSERT_FUN_LINE2}" 
                                         "${APPLICATION_ASSERT_FUN_LINE3}")
    set(INSTALL_EXCEPTION_HANDLERS "#define configINSTALL_EXCEPTION_HANDLERS 1")
else()
    set(APPLICATION_ASSERT_FUN 
    "void vApplicationAssert( const char *pcFile, uint32_t ulLine );")
    set(SETUP_TICK_FUN "void FreeRTOS_SetupTickInterrupt(void);")
    set(CONFIG_SETUP_TICK 
    "#define configSETUP_TICK_INTERRUPT() FreeRTOS_SetupTickInterrupt()")
    set(CLR_INTR_MASK_FROM_ISR 
    "#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x) vPortClearInterruptMask(x)")
    if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53") OR 
       ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR 
       ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5")
      )
        set(CMD_INT_MAX_OUTPUT_SIZE 
            "#define configCOMMAND_INT_MAX_OUTPUT_SIZE 2096")
        set(RECMU_CNTRL_TASK_PRIORITY 
        "#define recmuCONTROLLING_TASK_PRIORITY ( configMAX_PRIORITIES - 2 )")
        if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53") OR        
           ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") 
          )
       
            set(FABS_FUN "#define fabs( x ) __builtin_fabs( x )")
            set(SET_INT_MASK_FROM_ISR 
           "#define portSET_INTERRUPT_MASK_FROM_ISR() uxPortSetInterruptMask()") 
        endif()
    elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa9")
        set(SET_INT_MASK_FROM_ISR 
        "#define portSET_INTERRUPT_MASK_FROM_ISR() ulPortSetInterruptMask()")
    endif()
endif()

configure_file(${CMAKE_SOURCE_DIR}/FreeRTOSConfig.h.in ${CMAKE_BINARY_DIR}/include/FreeRTOSConfig.h)
