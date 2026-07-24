open_project E:/FPGA/RockDcCtrlTop/Dev/RockDcCtrlTop/RockDcCtrlTop.xpr
reset_run synth_1
launch_runs synth_1 -jobs 4
wait_on_run synth_1
set status [get_property STATUS [get_runs synth_1]]
puts "SYNTH_STATUS: $status"
if {$status != "synth_design Complete!"} {
    exit 1
}
exit 0
