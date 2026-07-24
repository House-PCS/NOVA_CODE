::------------------------------------------------------------------------------
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

::------------------------------------------------------------------------------

::------------------------------------------------------------------------------
::创建Temp文件夹
md Temp
RD Temp\work /s /q

::------------------------------------------------------------------------------

::------------------------------------------------------------------------------
::设置软件路径,设计代码路径,仿真代码路径
SET vsim=%MODELSIM_PATH%\vsim.exe
SET SRC_HOME=..\..\Src
SET SIM_HOME=.\
::SET PANGO_LIB=%PDS_PATH%\arch\vendor\pango\verilog\simulation

::------------------------------------------------------------------------------

::------------------------------------------------------------------------------
::生成.v的filelist
::导入仿真激励顶层.v文件
dir /s/b %SIM_HOME%\Bench\*.v>>Temp\verdi_filelist_v.f

::复制设计文件中IP的.v文件,并删除实例化*_tmpl.v文件
xcopy /s/e/i/y %SRC_HOME%\Core %SIM_HOME%\Ipcore
DEL /s %SIM_HOME%\Ipcore\*_tmpl.v

::导入仿真激励IP的仿真网表.v文件
dir /s/b %SIM_HOME%\Ipcore\*sim_netlist.v>>Temp\verdi_filelist_v.f

::导入仿真激励IP的仿真.v文件
::dir /s/b %SIM_HOME%\Ipcore\*.v>>Temp\verdi_filelist_v.f

::导入仿真模型Model的.v文件
dir /s/b %SIM_HOME%\Model\*.v>>Temp\verdi_filelist_v.f

::导入系统仿真SysSim的.v文件
dir /s/b %SIM_HOME%\SysSim\*.v>>Temp\verdi_filelist_v.f

::导入设计代码的.v文件
dir /s/b %SRC_HOME%\UserCode\*.v>>Temp\verdi_filelist_v.f

::导入设计代码中IP的网表.v文件
::dir /s/b %SRC_HOME%\Core\*sim_netlist.v>>Temp\verdi_filelist_v.f

::导入设计代码中IP的.v文件
::dir /s/b %SRC_HOME%\Core\*.v>>Temp\verdi_filelist_v.f

::导入PANGO的原语库.v文件
::dir /s/b %PANGO_LIB%\*.v>>Temp\verdi_filelist_v.f


::------------------------------------------------------------------------------

::------------------------------------------------------------------------------
::生成.sv的filelist
::导入仿真激励顶层.sv文件
dir /s/b %SIM_HOME%\Bench\*.sv>>Temp\verdi_filelist_sv.f

::导入仿真激励IP的仿真网表.sv文件
dir /s/b %SIM_HOME%\Ipcore\*sim_netlist.sv>>Temp\verdi_filelist_sv.f

::导入仿真模型Model的.sv文件
dir /s/b %SIM_HOME%\Model\*.sv>>Temp\verdi_filelist_sv.f

::导入系统仿真SysSim的.sv文件
dir /s/b %SIM_HOME%\SysSim\*.sv>>Temp\verdi_filelist_sv.f

::------------------------------------------------------------------------------

::------------------------------------------------------------------------------
::生成.c的filelist
::导入仿真激励顶层.c文件
dir /s/b %SIM_HOME%\Bench\*.c>>Temp\verdi_filelist_c.f

::导入仿真激励IP的仿真网表.c文件
dir /s/b %SIM_HOME%\Ipcore\*sim_netlist.c>>Temp\verdi_filelist_c.f

::导入仿真模型Model的.c文件
dir /s/b %SIM_HOME%\Model\*.c>>Temp\verdi_filelist_c.f

::导入系统仿真SysSim的.c文件
dir /s/b %SIM_HOME%\SysSim\*.c>>Temp\verdi_filelist_c.f

::------------------------------------------------------------------------------

::------------------------------------------------------------------------------
::生成.vp的filelist
::导入仿真模型Model的.vp文件
dir /s/b %SIM_HOME%\Model\*.vp>>Temp\verdi_filelist_vp.f

::------------------------------------------------------------------------------

::------------------------------------------------------------------------------
::将filelist.f中的反斜杠转为斜杠（便于vlog识别）
::verdi_filelist_v.f中文件路径修改反斜杠为斜杠
set fn=Temp\verdi_filelist_v.f
(
    for /f "tokens=1* delims=:" %%i in ('findstr/n .* "%fn%"')do (
        set "h=y%%j"
        setlocal enabledelayedexpansion
        set "h=!h:\=/!"
        echo.!h:~1!
        endlocal
    )
)>Temp\modelsim_filelist_v.f

::verdi_filelist_sv.f中文件路径修改反斜杠为斜杠
set fn=Temp\verdi_filelist_sv.f
(
    for /f "tokens=1* delims=:" %%i in ('findstr/n .* "%fn%"')do (
        set "h=y%%j"
        setlocal enabledelayedexpansion
        set "h=!h:\=/!"
        echo.!h:~1!
        endlocal
    )
)>Temp\modelsim_filelist_sv.f

::verdi_filelist_c.f中文件路径修改反斜杠为斜杠
set fn=Temp\verdi_filelist_c.f
(
    for /f "tokens=1* delims=:" %%i in ('findstr/n .* "%fn%"')do (
        set "h=y%%j"
        setlocal enabledelayedexpansion
        set "h=!h:\=/!"
        echo.!h:~1!
        endlocal
    )
)>Temp\modelsim_filelist_c.f

::verdi_filelist_vp.f中文件路径修改反斜杠为斜杠
set fn = Temp\verdi_filelist_vp.f
(
    for /f "tokens=1* delims=:" %%i in ('findstr/n .* "%fn%"')do (
        set "h=y%%j"
        setlocal enabledelayedexpansion
        set "h=!h:\=/!"
        echo.!h:~1!
        endlocal
    )
)>Temp\modelsim_filelist_vp.f

::------------------------------------------------------------------------------

::------------------------------------------------------------------------------
::调用MATLAB
::调用MATLAB参考模型，自动生成对比图片
::如果需要reference功能，需要在SysSim目录下
::新建Reference文件夹，然后打开下面注释

::move Temp.f "%fn%"

::cd /d SysSim\Reference
::start "matlab_cmd" cmd /c call TestRefMode.bat
::cd /d ..\..\

::------------------------------------------------------------------------------

::------------------------------------------------------------------------------
::调用Modelsim,同时删除临时文件Temp

::ModelSim Command（启动modelsim）
%vsim% -do script\Do.tcl -l Temp\simulate.log

::ModelSim Command（不启动modelsim）
::%vsim%  -c -do script\Do.tcl -l Temp\simulate.log

::删除ModelSim生成的相关文件


RD Ipcore\ /s /q
RD Temp\work /s /q
DEL Temp

::------------------------------------------------------------------------------

::------------------------------------------------------------------------------
::调用Debussy

::Debussy Command(自动打开debussy)
::%debussy% -f verdi_filelist.f -ssf wave.fsdb -2001

::删除波形文件
::DEL wave.fsdb /q

::删除Debussy生成的相关文件
::RD Debussy.exeLog  /s /q
::DEL debussy.rc /q

::退出命令行
pause
EXIT

::------------------------------------------------------------------------------
