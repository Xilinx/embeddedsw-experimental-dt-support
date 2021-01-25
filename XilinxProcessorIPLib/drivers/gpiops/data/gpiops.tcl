###############################################################################
# Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a sdm  11/22/11 Created
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XGpioPs" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

    ::hsi::utils::define_zynq_config_file $drv_handle "xgpiops_g.c" "XGpioPs"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_INTERRUPT" "C_INTR_PARENT"

    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XGpioPs" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR"

    gen_intr $drv_handle "xparameters.h"
}

proc gen_intr {drv_handle file_name} {
    set file_handle [::hsi::utils::open_include_file $file_name]
    set ips [::hsi::utils::get_common_driver_ips $drv_handle]
    set sw_processor [hsi::get_sw_processor]
    set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
    set processor_type [common::get_property IP_NAME $processor]

    foreach ip $ips {
        set isintr [::hsm::utils::is_ip_interrupting_current_proc $ip]
        set intc_parent_addr 0xffff
	set intr 0xffff
	if {$isintr == 1} {
            set intr_pin_name [hsi::get_pins -of_objects [hsi::get_cells -hier $ip]  -filter {TYPE==INTERRUPT&&DIRECTION==O}]
	    foreach pin $intr_pin_name {
		set intcname [::hsi::utils::get_connected_intr_cntrl $ip $pin]
                if {[llength $intcname] == 0 || [string match $intcname "{}"] } {
                    continue
                }
		foreach intc $intcname {
		    set ipname [common::get_property IP_NAME $intc]
		    if {$processor_type == "psu_cortexa53" && $ipname == "psu_acpu_gic"} {
		        set intc_parent_addr [common::get_property CONFIG.C_S_AXI_BASEADDR $intc]
		    }
		    if {$processor_type == "psu_cortexr5" && $ipname == "psu_rcpu_gic"} {
			set intc_parent_addr [common::get_property CONFIG.C_S_AXI_BASEADDR $intc]
		    }
		    if {$ipname == "axi_intc"} {
			set intc_parent_addr [common::get_property CONFIG.C_BASEADDR $intc]
			incr intc_parent_addr
			set intc_parent_addr [format 0x%xU $intc_parent_addr]
		    }
		}
            }
            if {${processor_type} == "microblaze"} {
		set intcname [string toupper $intcname]
		set ip_name [string toupper $ip]
		set intr_pin_name [string toupper $intr_pin_name]
	        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "C_INTERRUPT"] XPAR_${intcname}_${ip_name}_${intr_pin_name}_INTR"
	   } else {
                set ip_name [string toupper $ip]
	        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "C_INTERRUPT"] XPAR_${ip_name}_INTR"
	   }
	} else {
	        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "C_INTERRUPT"] $intr"
	}

        puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "C_INTR_PARENT"] $intc_parent_addr"
    }

    close $file_handle
}
