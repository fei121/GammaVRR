@echo off

rem ***************************************************************************************************************
rem READNE
rem 1. This bat is used to quickly generate dll or exe on the Windows platform, and copy the generated dll or exe to the corresponding path in "devPlatform".
rem 2. Configure the user parameters correctly, and the meaning of the parameters is explained by a comment at the top of the parameters.
rem 3. Before running "dll_build_and_move.bat", first open the VS project, configure whether you want to generate dll or exe in the project properties, and then close the VS project to save the settings.
rem 4. When the VS project settings generate dlls, runMode is set to 0. When the VS project generates an exe, runMode is set to 1.
rem 5. Double-click "dll_build_and_move.bat" to generate a dll or exe, and copy the generated dll or exe to the corresponding path in devPlatform.
rem Note:
rem 1. Before using the "dll_build_and_move.bat", make sure that the current VS project has the "x64" or "x86/Win32" platform, otherwise the dll or exe of the corresponding platform cannot be generated.
rem 2. If the current VS project does not include a certain platform, you can add "Add" in the "Configuration Manager".
rem ***************************************************************************************************************


rem ***************************************************************************************************************
rem User parameters
rem Please set the value of [runMode] [ipProjectName] [solutionName] [dstName] [msbuildPath] [devPlatformPath] [platformWin32]
rem ***************************************************************************************************************
rem 0: Build DLL; 1: Build EXE;
set runMode=0
rem The name of the current chip project;
set ipProjectName=HV7607B
rem The name of the Visual Studio project
set solutionName=GammaVRR
rem IP name in the dll or exe
set dstName=GammaVrr
rem The path of "MSBuild.exe" on the current PC, please confirm it on the current PC and set this path.
set msbuildPath=D:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\MSBuild\15.0\Bin\MSBuild.exe
rem The root directory of the "devPlatform", which is used to copy DLL or EXE.
set devPlatformPath=F:\00_sim_data\7607B_IP\DevPlatform
rem [x86] or [Win32]
set platformWin32=x86


rem ***************************************************************************************************************
rem Build dll or exe
rem Move to dst folder path
rem ***************************************************************************************************************
set solutionPath=%solutionName%.sln
set windowsExeSubPath=windows\exe_file\%ipProjectName%
set windowsDllSubPath=windows\dll\%ipProjectName%
set windowsExePath=%devPlatformPath%\%windowsExeSubPath%
set windowsDllPath=%devPlatformPath%\%windowsDllSubPath%
echo %windowsExePath%
echo %windowsDllPath%

rd /s /q Debug
rd /s /q Release
rd /s /q x64\Debug
rd /s /q x64\Release
if "%runMode%"=="0" (
	echo Build DLL...
	rem Configuration: Debug Release
	rem Platform:      [%platformWin32%] [x64]
	"%msbuildPath%" %solutionPath% /p:Configuration=Debug /p:Platform=%platformWin32%
	"%msbuildPath%" %solutionPath% /p:Configuration=Release /p:Platform=%platformWin32%
	"%msbuildPath%" %solutionPath% /p:Configuration=Debug /p:Platform=x64
	"%msbuildPath%" %solutionPath% /p:Configuration=Release /p:Platform=x64
	rem Copy exe and dll to devPlatformPath
	copy Debug\%solutionName%.dll              %windowsDllPath%\%ipProjectName%_%dstName%_Win32_Debug.dll
	copy Release\%solutionName%.dll            %windowsDllPath%\%ipProjectName%_%dstName%_Win32_Release.dll
	copy x64\Debug\%solutionName%.dll          %windowsDllPath%\%ipProjectName%_%dstName%_x64_Debug.dll
	copy x64\Release\%solutionName%.dll        %windowsDllPath%\%ipProjectName%_%dstName%_x64_Release.dll
) else if "%runMode%"=="1" (
    echo Build EXE...
	"%msbuildPath%" %solutionPath% /p:Configuration=Release /p:Platform=%platformWin32%
	copy Release\%solutionName%.exe            %windowsExePath%\%ipProjectName%_%dstName%.exe
) else (
    echo error: runMode is only allowed to be 0 or 1
    exit /b 1
)

echo Build process completed.

pause