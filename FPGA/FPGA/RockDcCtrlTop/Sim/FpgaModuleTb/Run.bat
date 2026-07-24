::关闭回显
@ECHO OFF
::删除之前生成的filelist.f
DEL Temp\verdi_filelist_v.f
DEL Temp\verdi_filelist_sv.f
DEL Temp\verdi_filelist_c.f
DEL Temp\verdi_filelist_vp.f
DEL Temp\modelsim_filelist_v.f
DEL Temp\modelsim_filelist_sv.f
DEL Temp\modelsim_filelist_c.f
DEL Temp\modelsim_filelist_vp.f

md Temp
RD Temp\work /s /q
::设置软件路径
::SET debussy=D:\Novas\Debussy\bin\Debussy.exe
SET vsim=%MODELSIM_PATH%\vsim.exe
SET SRC_HOME=.\Src
SET SIM_HOME=.\

::_________________________________________________
::生成filelist,分为Verilog\systemverilog\c
::导入本次case的所有.v和sv文件
::导入专用仿真模型model的所有.v和sv文件
::导入设计中除顶层外的所有.v和sv文件
::导入tb中的C文件，设计中不允许存在C文件
::_________________________________________________
dir /s/b %SIM_HOME%\Bench\*.v >>Temp\verdi_filelist_v.f
dir /s/b %SIM_HOME%\Ipcore\*sim_netlist.v >>Temp\verdi_filelist_v.f
dir /s/b %SIM_HOME%\Model\*.v >>Temp\verdi_filelist_v.f
dir /s/b %SIM_HOME%\SysSim\*.v >>Temp\verdi_filelist_v.f

::for /f "delims=" %%I in ('dir /s/b %SRC_HOME%\UserCode\*.v ^|find /v "Src\Core"') do (echo %%I >>Temp\verdi_filelist_v.f)

dir /s/b %SRC_HOME%\SimCode\*.v >>Temp\verdi_filelist_v.f
dir /s/b %SRC_HOME%\Core\*sim_netlist.v >>Temp\verdi_filelist_v.f
dir /s/b %SRC_HOME%\UserCode\*.v >>Temp\verdi_filelist_v.f

dir /s/b %SIM_HOME%\Bench\*.sv >>Temp\verdi_filelist_sv.f
dir /s/b %SIM_HOME%\Ipcore\*sim_netlist.sv >>Temp\verdi_filelist_sv.f
dir /s/b %SIM_HOME%\Model\*.sv >>Temp\verdi_filelist_sv.f
dir /s/b %SIM_HOME%\SysSim\*.sv >>Temp\verdi_filelist_sv.f


dir /s/b %SIM_HOME%\Bench\*.c >>Temp\verdi_filelist_c.f
dir /s/b %SIM_HOME%\Ipcore\*sim_netlist.c >>Temp\verdi_filelist_c.f
dir /s/b %SIM_HOME%\Model\*.c >>Temp\verdi_filelist_c.f
dir /s/b %SIM_HOME%\SysSim\*.c >>Temp\verdi_filelist_c.f

dir /s/b %SIM_HOME%\Model\*.vp >>Temp\verdi_filelist_vp.f


::________________________________________________
::将filelist.f中的反斜杠转为斜杠（便于vlog识别）
::________________________________________________
set fn=Temp\verdi_filelist_v.f
(for /f "tokens=1* delims=:" %%i in ('findstr/n .* "%fn%"')do (
set "h=y%%j"
setlocal enabledelayedexpansion
set "h=!h:\=/!"
echo.!h:~1!
endlocal))>Temp\modelsim_filelist_v.f

set fn=Temp\verdi_filelist_sv.f
(for /f "tokens=1* delims=:" %%i in ('findstr/n .* "%fn%"')do (
set "h=y%%j"
setlocal enabledelayedexpansion
set "h=!h:\=/!"
echo.!h:~1!
endlocal))>Temp\modelsim_filelist_sv.f

set fn=Temp\verdi_filelist_c.f
(for /f "tokens=1* delims=:" %%i in ('findstr/n .* "%fn%"')do (
set "h=y%%j"
setlocal enabledelayedexpansion
set "h=!h:\=/!"
echo.!h:~1!
endlocal))>Temp\modelsim_filelist_c.f

set fn=Temp\verdi_filelist_vp.f
(for /f "tokens=1* delims=:" %%i in ('findstr/n .* "%fn%"')do (
set "h=y%%j"
setlocal enabledelayedexpansion
set "h=!h:\=/!"
echo.!h:~1!
endlocal))>Temp\modelsim_filelist_vp.f

::move Temp.f "%fn%"
::_______________________________________________
:: 调用MATLAB参考模型，自动生成对比图片
:: 如果需要reference功能，需要在SysSim目录下
:: 新建Reference文件夹，然后打开下面注释
::_______________________________________________
::cd /d SysSim\Reference
::start "matlab_cmd" cmd /c call TestRefMode.bat
::cd /d ..\..\


::_______________________________________________
::调用modelsim，同时删除临时文件Temp
::_______________________________________________
::ModelSim Command（启动modelsim）
::%vsim% -do script\do.tcl -l Temp\simulate.log

::ModelSim Command（不启动modelsim）
%vsim%  -c -do script\Do.tcl -l Temp\simulate.log

::删除ModelSim生成的相关文件
RD Temp\work /s /q
DEL Temp
::Debussy Command(自动打开debussy)
::%debussy% -f verdi_filelist.f -ssf wave.fsdb -2001
::删除波形文件
::DEL wave.fsdb /q
::删除Debussy生成的相关文件
::RD Debussy.exeLog  /s /q
::DEL debussy.rc /q
pause
::退出命令行
EXIT
