###############################################################################
# Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.0   hk  08/21/14 First release
#       sk  05/06/15 Imported Bus Width Parameter.
# 1.5	nsk 08/14/17 Added CCI support
# 1.9   mus 07/30/19 Added CCI support for Versal at EL1 NS
# 1.11	sd  27/03/20 Added Clock support
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    ::hsi::utils::define_zynq_include_file $drv_handle "xparameters.h" "XQspiPsu" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_QSPI_CLK_FREQ_HZ" "C_QSPI_MODE" "C_QSPI_BUS_WIDTH"
    generate_cci_params $drv_handle "xparameters.h"

	set clocking [common::get_property CONFIG.clocking [hsi::get_os]]
	set is_zynqmp_fsbl_bsp [common::get_property CONFIG.ZYNQMP_FSBL_BSP [hsi::get_os]]
	set cortexa53proc [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa53"}]
	set isclocking [check_clocking]

	if { $isclocking == 1 &&  $is_zynqmp_fsbl_bsp != true   &&  [llength $cortexa53proc] > 0 && [string match -nocase $clocking "true"] > 0} {
    ::hsi::utils::define_zynq_config_file $drv_handle "xqspipsu_g.c" "XQspiPsu"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_QSPI_CLK_FREQ_HZ" "C_QSPI_MODE" "C_QSPI_BUS_WIDTH" "IS_CACHE_COHERENT" "REF_CLK" "C_INTERRUPT" "C_INTR_PARENT"
	} else {
    ::hsi::utils::define_zynq_config_file $drv_handle "xqspipsu_g.c" "XQspiPsu"  "DEVICE_ID" "C_S_AXI_BASEADDR" "C_QSPI_CLK_FREQ_HZ" "C_QSPI_MODE" "C_QSPI_BUS_WIDTH" "IS_CACHE_COHERENT" "C_INTERRUPT" "C_INTR_PARENT"
	}
    ::hsi::utils::define_zynq_canonical_xpars $drv_handle "xparameters.h" "XQspiPsu" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_QSPI_CLK_FREQ_HZ" "C_QSPI_MODE" "C_QSPI_BUS_WIDTH" "IS_CACHE_COHERENT"

    gen_intr $drv_handle "xparameters.h"
}

proc check_clocking { } {
	set sw_proc_handle [hsi::get_sw_processor]
	set slaves [common::get_property   SLAVES [  hsi::get_cells -hier $sw_proc_handle]]
	foreach slave $slaves {
		if {[string compare -nocase "psu_crf_apb" $slave] == 0 } {
			return 1
		}
	}
	return 0
}

proc generate_cci_params {drv_handle file_name} {
	set file_handle [::hsi::utils::open_include_file $file_name]
	# Get all peripherals connected to this driver
	set ips [::hsi::utils::get_common_driver_ips $drv_handle]

	set sw_processor [hsi::get_sw_processor]
	set processor [hsi::get_cells -hier [common::get_property HW_INSTANCE $sw_processor]]
	set processor_type [common::get_property IP_NAME $processor]
	set isclocking [check_clocking]

	foreach ip $ips {
		set cci_enble 0
		set ref_tag 0xff
		if {$processor_type == "psu_cortexa53"} {
			set hypervisor [common::get_property CONFIG.hypervisor_guest [hsi::get_os]]
			set ref_tag  "QSPI_REF"
			if {[string match -nocase $hypervisor "true"]} {
				set cci_enble [common::get_property CONFIG.IS_CACHE_COHERENT $ip]
			}
		} elseif {$processor_type == "psv_cortexa72"} {
			set extra_flags [common::get_property CONFIG.extra_compiler_flags [hsi::get_sw_processor]]
			set flagindex [string first {-DARMA72_EL3} $extra_flags 0]
			if {$flagindex == -1} {
				set cci_enble [common::get_property CONFIG.IS_CACHE_COHERENT $ip]
			}
		}
		puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "IS_CACHE_COHERENT"] $cci_enble"
		if { $isclocking == 1 } {
			puts $file_handle "\#define [::hsi::utils::get_driver_param_name $ip "REF_CLK"] $ref_tag"
		}
	}
	close $file_handle
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
