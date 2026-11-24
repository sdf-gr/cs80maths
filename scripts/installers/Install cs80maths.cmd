@echo off
setlocal
if "%CS80MATHS_TEST_ROOT%"=="" (set "base=%LOCALAPPDATA%\Programs\Common\VST3\Direct Grooves") else (set "base=%CS80MATHS_TEST_ROOT%\VST3\Direct Grooves")
set "target=%base%\cs80maths.vst3"
echo Close your DAW before installing cs80maths.
if not exist "%~dp0cs80maths.vst3\Contents\x86_64-win\cs80maths.vst3" (echo Missing plugin bundle. Extract the full ZIP first. & exit /b 1)
if exist "%target%" (
 if not exist "%target%\.directgrooves-cs80maths" (echo Unrecognized existing destination. Nothing changed. & exit /b 1)
 if exist "%target%.previous" (echo A previous backup already exists. Move it elsewhere before updating. & exit /b 1)
 move "%target%" "%target%.previous" >nul || exit /b 1
)
if not exist "%base%" mkdir "%base%" || exit /b 1
xcopy "%~dp0cs80maths.vst3" "%target%\" /E /I /H /Y /Q >nul || exit /b 1
>"%target%\.directgrooves-cs80maths" echo Direct Grooves cs80maths
copy /Y "%~dp0Uninstall cs80maths.cmd" "%base%\Uninstall cs80maths.cmd" >nul || exit /b 1
echo Installed: %target%
echo Restart/rescan your VST3 host. If needed, add %LOCALAPPDATA%\Programs\Common\VST3 to its plugin scan paths.
echo Uninstaller included here and in the extracted download. User presets and settings are kept.
if not "%CS80MATHS_TEST_MODE%"=="1" pause
exit /b 0
