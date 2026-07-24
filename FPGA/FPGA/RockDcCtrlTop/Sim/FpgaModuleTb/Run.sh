#!/bin/bash
### Start Matlab ###
#_______________________________________________
# 调用MATLAB参考模型，自动生成对比图片
# 如果需要reference功能，需要在SysSim目录下
# 新建Reference文件夹，然后打开下面注释
#_______________________________________________
#cd SysSim/Reference/
#chmod 777 TestRefMode.sh
#./TestRefMode.sh
#cd ../../

#=========================================================================

### Start Questasim ###
mkdir Temp
rm -rf Temp/work
rm -rf Temp/modelsim_filelist_h.f
rm -rf Temp/modelsim_filelist_v.f
rm -rf Temp/modelsim_filelist_sv.f
rm -rf Temp/modelsim_filelist_c.f
path=$PWD
MAIN_SRC_PATH=$path/../Src
MAIN_IPCORE_PATH=$MAIN_SRC_PATH/Ipcore
MAIN_BD_PATH=$MAIN_SRC_PATH/Bd

find $path/Macro/ -name "*.h" >Temp/modelsim_filelist_h.f

find $MAIN_SRC_PATH \( -path $MAIN_IPCORE_PATH -o -path $MAIN_BD_PATH \) -prune -o -type f -name "*.v" -print >Temp/modelsim_filelist_v.f
find $path/Bench/ -name "*.v" >>Temp/modelsim_filelist_v.f
find $path/Ipcore/ -name "*sim_netlist.v" >>Temp/modelsim_filelist_v.f
find $path/Model/ -name "*.v" >>Temp/modelsim_filelist_v.f
find $path/SysSim/  -name "*.v" >>Temp/modelsim_filelist_v.f
find $MAIN_IPCORE_PATH -name "*sim_netlist.v" >>Temp/modelsim_filelist_v.f

find $MAIN_SRC_PATH \( -path "$MAIN_IPCORE_PATH" -o -path "$MAIN_BD_PATH" \) -prune -o -type f -name "*.sv" -print >Temp/modelsim_filelist_sv.f
find $path/Bench/ -name "*.sv" >>Temp/modelsim_filelist_sv.f
find $path/Model/ -name "*.sv" >>Temp/modelsim_filelist_sv.f
find $path/SysSim/  -name "*.sv" >>Temp/modelsim_filelist_sv.f

find $MAIN_SRC_PATH \( -path "$MAIN_IPCORE_PATH" -o -path "$MAIN_BD_PATH" \) -prune -o -type f -name "*.c" -print >Temp/modelsim_filelist_c.f
find $path/Bench/ -name "*.c" >>Temp/modelsim_filelist_c.f
find $path/Model/ -name "*.c" >>Temp/modelsim_filelist_c.f
find $path/SysSim/  -name "*.c" >>Temp/modelsim_filelist_c.f

find $MAIN_SRC_PATH \( -path "$MAIN_IPCORE_PATH" -o -path "$MAIN_BD_PATH" \) -prune -o -type f -name "*.sv" -print >Temp/modelsim_filelist_vp.f
find $path/Model/ -name "*.vp" >>Temp/modelsim_filelist_vp.f

vsim -c -do Script/Do.tcl -l Temp/simulate.log
