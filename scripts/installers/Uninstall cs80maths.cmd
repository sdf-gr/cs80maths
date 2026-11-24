@echo off
setlocal
if "%CS80MATHS_TEST_ROOT%"=="" (set "base=%LOCALAPPDATA%\Programs\Common\VST3\Direct Grooves") else (set "base=%CS80MATHS_TEST_ROOT%\VST3\Direct Grooves")
set "target=%base%\cs80maths.vst3"
echo Close your DAW before uninstalling. Presets and settings will be kept.
if not "%CS80MATHS_TEST_MODE%"=="1" (
 set /p "reply=Type YES to uninstall cs80maths: "
)
if not "%CS80MATHS_TEST_MODE%"=="1" if /I not "%reply%"=="YES" exit /b 0
if not exist "%target%" (echo cs80maths is not installed at the managed location. & exit /b 0)
if not exist "%target%\.directgrooves-cs80maths" (echo Unrecognized destination. Nothing removed. & exit /b 1)
rmdir /S /Q "%target%" || exit /b 1
echo Removed cs80maths VST3. Restart/rescan your DAW.
echo User presets, settings, earlier backups and manually installed copies were left alone.
if not "%CS80MATHS_TEST_MODE%"=="1" pause
exit /b 0
