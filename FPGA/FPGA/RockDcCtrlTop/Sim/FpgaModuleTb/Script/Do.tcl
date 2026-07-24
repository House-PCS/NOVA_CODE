# set glbl_home $env(XILINX_VIVADO)/data/verilog/src
set glbl_home $env(XILINX_VIVADO)/ISE_DS/ISE/verilog/src
#设置源文件目录
set src_home ./Src
set bench_home ./Bench
set macro_sim ./Macro
set ddr_home ./Model/DdrModel

#_____________________________________________#
#生成当前仿真时间，并将时间作为wlf波形的名称
#_____________________________________________#
set curtime [clock seconds]
set change_date [string map {"/" "_"} [clock format $curtime -format {%D}]]
puts $change_date
set change_time [string map {":" "_"} [clock format $curtime -format {%T}]]
puts $change_time
set link "-"
set formattime $change_date$link$change_time

#_____________________________________________#
#找到bench目录下的.v文件，并获取其名称作为tb_name
#_____________________________________________#
if {[file exists [glob -nocomplain $bench_home/*.v]] == 1} {
    proc next_dir {cur_dir} {glob -nocomplain -directory $cur_dir -types f *.v}
} else {
    proc next_dir {cur_dir} {glob -nocomplain -directory $cur_dir -types f *.sv}
}
set path [next_dir $bench_home]
set tb_name [lindex [split [file tail $path] .] 0]

#新建work库
vlib Temp/work
#映射默认work库到sim home下的work库
vmap work Temp/work

#_____________________________________________#
#检测顶层代码是否存在，不存在提前报错，存在则
#vlog编译，同时编译filelist中的.v和.sv
#_____________________________________________#
if {[file exists [glob -nocomplain $src_home/UserCode/*.v]] == 1} {
    vlog +incdir+$macro_sim +define+SIMULATION_EN $src_home/UserCode/*.v
} else {
    puts "================================================================\n
        Error,Error,Error, The Top design is not exist!!!
================================================================\n"
    exit 
}

if {[file size Temp/modelsim_filelist_v.f] >0} {
    vlog +incdir+$macro_sim +define+SIMULATION_EN -f Temp/modelsim_filelist_v.f
}

if {[file size Temp/modelsim_filelist_vp.f] >0} {
    vlog +incdir+$macro_sim +define+SIMULATION_EN -f Temp/modelsim_filelist_vp.f
}

if {[file size Temp/modelsim_filelist_sv.f] >0} {
    vlog -sv +incdir+$macro_sim +define+SIMULATION_EN -f Temp/modelsim_filelist_sv.f
}

if {[file size Temp/modelsim_filelist_c.f] >0} {
    vlog -f Temp/modelsim_filelist_c.f
}

#编译DDR库文件
#vlog -sv +incdir+$ddr_home +define+x2Gb +define+sg107E +define+x16 $ddr_home/ddr3_model.sv

#编译glbl.v
file copy -force $glbl_home/glbl.v ./Temp/
vlog ./Temp/glbl.v

#modelsim编译库
# vsim -wlf WaveFile/WlfFile/wave_$formattime.wlf -voptargs="+acc" -L work -L unisims_ver -L unimacro_ver -L secureip -L xpm -lib work work.$tb_name work.glbl
vsim -wlf WaveFile/WlfFile/wave_$formattime.wlf -voptargs="+acc" -L work -lib work work.$tb_name work.glbl
#vsim -pli novas_fli.so -voptargs="+acc" -L work -L unisims_ver -L unimacro_ver -L secureip -L xpm -lib work work.$tb_name work.glbl

#用于保存全部波形信息
log -r /*
log -r /tb/FiberCvrt64B66BTopInst/FiberCvrtTxTopInst/FifoRdEn  
log -r /tb/FiberCvrt64B66BTopInst/FiberCvrtTxTopInst/FifoRdVld 
log -r /tb/FiberCvrt64B66BTopInst/FiberCvrtTxTopInst/FifoRdCnt 
log -r /tb/FiberCvrt64B66BTopInst/FiberCvrtTxTopInst/FifoRdData
log -r /tb/FiberCvrt64B66BTopInst/FiberCvrtTxTopInst/FifoEmpty 


#用于保存指定模块的波形信息
#log -r /tb_CVT10/DutRx/FiberCvrtTopInst/*
#log -r /tb_CVT10/*

#执行仿真
run -all

##打开波形窗口
#run 100ms
#puts "**************************************************
#                the simulation is over!!
#**************************************************"



