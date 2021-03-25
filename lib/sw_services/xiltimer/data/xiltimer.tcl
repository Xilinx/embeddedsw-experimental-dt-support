###############################################################################
#
# Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
###############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
##############################################################################

set sleep_timer_is_ttc 0
set sleep_timer_is_axitimer 0
set sleep_timer_is_scutimer 0
set sleep_timer_is_default 0
set tick_timer_is_default 0

#---------------------------------------------
# timer_drc
#---------------------------------------------
proc timer_drc {lib_handle} {

}

proc xtimer_drc {lib_handle} {
	global sleep_timer_is_ttc
	global sleep_timer_is_axitimer
	global sleep_timer_is_scutimer
	global sleep_timer_is_default
	global tick_timer_is_default

	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]

	set proc_type [common::get_property IP_NAME [hsi::get_cells -hier $hw_processor]];
        puts "proc_type $proc_type"
	set default_dir "src/core/defalut/"
	set ttcps_dir "src/core/ttcps/"
	set axitmr_dir "src/core/axi_timer/"
	set scutimer_dir "src/core/scutimer/"

	puts "sleep_timer_is_ttc $sleep_timer_is_ttc"
	
	if {$sleep_timer_is_ttc != 0} {
		foreach entry [glob -nocomplain -types f [file join $ttcps_dir *]] {
			file copy -force $entry "./src"
        	}
	}
	if {$sleep_timer_is_axitimer != 0} {
		foreach entry [glob -nocomplain -types f [file join $axitmr_dir *]] {
			file copy -force $entry "./src"
        	}
	}
	if {$sleep_timer_is_scutimer != 0} {
		foreach entry [glob -nocomplain -types f [file join $scutimer_dir *]] {
			file copy -force $entry "./src"
        	}
	}
	if {$sleep_timer_is_default != 0 || $tick_timer_is_default != 0} {
		if {$proc_type == "psu_cortexa53" || $proc_type == "psv_cortexa72"} {
			file copy -force "src/core/default_timer/globaltimer_sleep.c" "./src"
		}
		if {$proc_type == "psu_cortexr5" || $proc_type == "psv_cortexr5"} {
			file copy -force "src/core/default_timer/cortexr5_sleep.c" "./src"
		}
		if {$proc_type == "microblaze" || $proc_type == "psu_pmu"
			|| $proc_type == "psv_pmc" || $proc_type == "psv_psm"} {
			file copy -force "src/core/default_timer/microblaze_sleep.c" "./src"
		}
		if {$proc_type == "ps7_cortexa9"} {
			file copy -force "src/core/default_timer/globaltimer_sleep_zynq.c" "./src"
		}
	}
	file delete -force ./src/core
}

proc generate {lib_handle} {
	global sleep_timer_is_ttc
	global sleep_timer_is_axitimer
	global sleep_timer_is_scutimer
	global sleep_timer_is_default
	global tick_timer_is_default

        set sleep_timer_is_ttc 0
        set sleep_timer_is_axitimer 0 
        set sleep_timer_is_scutimer 0
	set sleep_timer_is_default 0
	set tick_timer_is_default 0

	set sleep_timer [common::get_property CONFIG.sleep_timer $lib_handle]
	set tick_timer [common::get_property CONFIG.tick_timer $lib_handle]
        # for tick functionality interrupt connection is manadatory
	set ttc_ips [hsi::get_cells -hier -filter {IP_NAME == "psv_ttc"  || IP_NAME == "psu_ttc" || IP_NAME == "ps7_ttc"}]
	set axitmr_ips [hsi::get_cells -hier -filter {IP_NAME == "axi_timer"}]
	set scutmr_ips [hsi::get_cells -hier -filter {IP_NAME == "ps7_scutimer"}]
	set timer_ips [concat $ttc_ips $axitmr_ips $scutmr_ips]
        puts "timer_ips $timer_ips"
        set timer_len [llength $timer_ips]
        puts "timer_len $timer_len"
	# check processor type
	set proc_instance [hsi::get_sw_processor];
	set hw_processor [common::get_property HW_INSTANCE $proc_instance]
	set is_zynqmp_fsbl_bsp [common::get_property CONFIG.ZYNQMP_FSBL_BSP [hsi::get_os]]
	set cortexa53proc [hsi::get_cells -hier -filter {IP_NAME=="psu_cortexa53_0"}]
	set is_ticktimer_en [common::get_property CONFIG.en_tick_timer $lib_handle]

	if {$proc_instance == "psv_pmc_0" || $proc_instance == "psu_pmu_0" || $proc_instance == "psv_psm_0"} {
		incr sleep_timer_is_default
		incr tick_timer_is_default
	} elseif {$proc_instance == "psu_cortexa53_0" && $is_zynqmp_fsbl_bsp == true} {
		incr sleep_timer_is_default
		incr tick_timer_is_default
	} elseif { [expr [llength $timer_ips] >= 2] } {
		if { $sleep_timer == "none"} {
			set sleep_timer [lindex $timer_ips 0]
		}
		puts "is_ticktimer_en $is_ticktimer_en"
		if { $is_ticktimer_en} {
			if { $tick_timer == "none"} {
				set tick_timer [lindex $timer_ips 1]
			}
		} elseif { $tick_timer == "none"} {
			incr tick_timer_is_default
		}
	} elseif {[expr [llength $timer_ips] != 0]} {
		set sleep_timer [lindex $timer_ips 0]
		incr tick_timer_is_default
	} else {
		incr sleep_timer_is_default 
		incr tick_timer_is_default
		puts "No timer IP's available in the design"
	}

	set tmrcfg_file "src/xtimer_config.h"
	set fd [open $tmrcfg_file w]
	puts $fd "\#ifndef _XTIMER_CONFIG_H"
	puts $fd "\#define _XTIMER_CONFIG_H"
	puts $fd ""
	puts $fd "#include \"xparameters.h\""
	puts $fd ""
	if {[llength $ttc_ips] != 0} {
		if {[lsearch -exact $ttc_ips $sleep_timer] >= 0} {
		      puts $fd "\#define XSLEEPTIMER_IS_TTCPS"
		      set ipname [string toupper $sleep_timer]
		      puts $fd "\#define XSLEEPTIMER_DEVICEID XPAR_${ipname}_DEVICE_ID"
		      incr sleep_timer_is_ttc
		}
		if {[lsearch -exact $ttc_ips $tick_timer] >= 0} {
		      puts $fd "\#define XTICKTIMER_IS_TTCPS"
		      set ipname [string toupper $tick_timer]
		      puts $fd "\#define XTICKTIMER_DEVICEID XPAR_${ipname}_DEVICE_ID"
		      incr sleep_timer_is_ttc
		}
	}	
	if {[llength $axitmr_ips] != 0} {
		if {[lsearch -exact $axitmr_ips $sleep_timer] >= 0} {
		      puts $fd "\#define XSLEEPTIMER_IS_AXITIMER"
		      set ipname [string toupper $sleep_timer]
		      puts $fd "\#define XSLEEPTIMER_DEVICEID XPAR_${ipname}_DEVICE_ID"
		      incr sleep_timer_is_axitimer
		}
		if {[lsearch -exact $axitmr_ips $tick_timer] >= 0} {
		      puts $fd "\#define XTICKTIMER_IS_AXITIMER"
		      set ipname [string toupper $tick_timer]
		      puts $fd "\#define XTICKTIMER_DEVICEID XPAR_${ipname}_DEVICE_ID"
		      incr sleep_timer_is_axitimer
		}
	}	
	if {[llength $scutmr_ips] != 0} {
		if {[lsearch -exact $scutmr_ips $sleep_timer] >= 0} {
		      puts $fd "\#define XSLEEPTIMER_IS_SCUTIMER"
		      set ipname [string toupper $sleep_timer]
		      puts $fd "\#define XSLEEPTIMER_DEVICEID XPAR_${ipname}_DEVICE_ID"
		      incr sleep_timer_is_scutimer
		}
		if {[lsearch -exact $scutmr_ips $tick_timer] >= 0} {
		      puts $fd "\#define XTICKTIMER_IS_SCUTIMER"
		      set ipname [string toupper $tick_timer]
		      puts $fd "\#define XTICKTIMER_DEVICEID XPAR_${ipname}_DEVICE_ID"
		      incr sleep_timer_is_scutimer
		}
	}
	if {$sleep_timer_is_default != 0} {
		puts $fd "\#define XTIMER_IS_DEFAULT_TIMER" 
	}
	if {$tick_timer_is_default != 0} {
		puts $fd "\#define XTIMER_NO_TICK_TIMER"
	}
	puts $fd ""
	puts $fd "\#endif /* XTIMER_CONFIG_H */"
	close $fd
	xtimer_drc $lib_handle	
}
