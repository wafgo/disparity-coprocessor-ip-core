open_project vivado-hls-project
set_top disparity_pixel_coprocessor
add_files disparity_core.cpp
add_files -tb test_disparity_coprocessor.cpp
open_solution "solution1"
set_part {xc7z020clg484-1}
create_clock -period 10 -name default
#source "directives.tcl"
csim_design
csynth_design
cosim_design -rtl vhdl
export_design -format ip_catalog
