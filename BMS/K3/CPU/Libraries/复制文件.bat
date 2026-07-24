@echo off
setlocal enabledelayedexpansion

for %%F in (*) do (
    if not "%%F"=="%~nx0" (
        set "filename=%%~nF"
        set "extension=%%~xF"
        set "newfile=!filename!1!extension!"
        copy "%%F" "!newfile!" >nul
        echo 已创建: !newfile!
    )
)

echo 操作完成！
pause