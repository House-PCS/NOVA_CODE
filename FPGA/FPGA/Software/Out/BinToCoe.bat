call ..\Doc\BinToCoe\main.exe
copy ".\*.bin" "..\Out\"
copy ".\*.coe" "..\Out\BramInitFile\"
del ".\*.bin"
del ".\*.coe"