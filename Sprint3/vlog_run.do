quietly set ACTELLIBNAME PolarFire
quietly set PROJECT_DIR "C:/FPGA/PF_LED_blink"
quietly set INSTALL_DIR "C:/Microchip/Libero_SoC_v2023.1"

if {[file exists presynth/_info]} {
   echo "INFO: Simulation library presynth already exists"
} else {
   file delete -force presynth 
   vlib presynth
}
vmap presynth presynth
vmap PolarFire "${INSTALL_DIR}/Designer/lib/modelsimpro/precompiled/vlog/PolarFire"

vlog -vlog01compat -work presynth "${PROJECT_DIR}/component/work/PF_CLK_DIV_C0/PF_CLK_DIV_C0_0/PF_CLK_DIV_C0_PF_CLK_DIV_C0_0_PF_CLK_DIV.v"
vlog -vlog01compat -work presynth "${PROJECT_DIR}/component/work/PF_CLK_DIV_C0/PF_CLK_DIV_C0.v"
vlog -vlog01compat -work presynth "${PROJECT_DIR}/hdl/LED_ctrl.v"
vlog -vlog01compat -work presynth "${PROJECT_DIR}/component/work/PF_OSC_C0/PF_OSC_C0_0/PF_OSC_C0_PF_OSC_C0_0_PF_OSC.v"
vlog -vlog01compat -work presynth "${PROJECT_DIR}/component/work/PF_OSC_C0/PF_OSC_C0.v"
vlog -vlog01compat -work presynth "${PROJECT_DIR}/component/work/Fabric_Top/Fabric_Top.v"
vlog "+incdir+${PROJECT_DIR}/stimulus" -vlog01compat -work presynth "${PROJECT_DIR}/stimulus/user_testbench.v"


vsim -L PolarFire -L presynth  -t 1ps presynth.testbench
do "wave.do"

run 42 us
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(12) 1 0
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(11) 1 0
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(10) 1 0
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(9) 1 0
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(8) 1 0
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(7) 1 0
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(6) 1 0
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(5) 1 0
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(4) 1 0
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(3) 1 0
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(2) 1 0
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(1) 1 0
force -freeze sim:/testbench/Fabric_Top_0/LED_ctrl_0/counter(0) 1 0

run 2458 us
