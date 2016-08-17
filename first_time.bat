call "%VS110COMNTOOLS%"\vsvars32.bat
set QUICKSLINGPATH=%~dp0
if not exist "%QUICKSLINGPATH%\cef_binary\CURRENT_VERSION_3.2526.1347.gcf20046" (
	GOTO dostuff
)

if not exist "%QUICKSLINGPATH%\cef_binary\Debug\libcef.dll" (
	GOTO dostuff
)

if not exist "%QUICKSLINGPATH%\cef_binary\build\libcef_dll\Debug\libcef_dll_wrapper.lib" (
	GOTO compilestuff
)

if not exist "%QUICKSLINGPATH%\cef_binary\build\libcef_dll\Release\libcef_dll_wrapper.lib" (
	GOTO compilestuff
)

GOTO END

:dostuff
	if not exist "%QUICKSLINGPATH%\archives" (
		mkdir "%QUICKSLINGPATH%\archives"
	)

	if not exist "%QUICKSLINGPATH%\archives\cef_binary_3.2526.1347.gcf20046_windows32.7z" ( 
		"%QUICKSLINGPATH%\tools\wget" https://storage.googleapis.com/quicksling/build/cef_binary_3.2526.1347.gcf20046_windows32.7z -O "%QUICKSLINGPATH%archives\cef_binary_3.2526.1347.gcf20046_windows32.7z"
	)
	
	"%QUICKSLINGPATH%\tools\7za.exe" x -y -o"%QUICKSLINGPATH%" "archives\cef_binary_3.2526.1347.gcf20046_windows32.7z" "cef_binary_3.2526.1347.gcf20046_windows32\Debug" "cef_binary_3.2526.1347.gcf20046_windows32\Release"
	robocopy "cef_binary_3.2526.1347.gcf20046_windows32\Debug" "%QUICKSLINGPATH%\cef_binary\Debug" /E /IS /MOVE
	robocopy "cef_binary_3.2526.1347.gcf20046_windows32\Release" "%QUICKSLINGPATH%\cef_binary\Release" /E /IS /MOVE
	rmdir /S /Q "%QUICKSLINGPATH%\cef_binary_3.2526.1347.gcf20046_windows32"

	if exist "%QUICKSLINGPATH%\cef_binary\build" (
		rmdir /S /Q "%QUICKSLINGPATH%\cef_binary\build"
	)

	if not exist "%QUICKSLINGPATH%\cef_binary\build" (
		mkdir "%QUICKSLINGPATH%\cef_binary\build"
	)

	cmake -G "Visual Studio 12" -B"%QUICKSLINGPATH%\cef_binary\build" -H"%QUICKSLINGPATH%\cef_binary\"

	set CMAKE_ERRORLEVEL=%ERRORLEVEL%
	if "%CMAKE_ERRORLEVEL%" EQU "0" type nul >> "%QUICKSLINGPATH%\cef_binary\CURRENT_VERSION_3.2526.1347.gcf20046" & copy "%QUICKSLINGPATH%\cef_binary\CURRENT_VERSION_3.2526.1347.gcf20046" +,,
:compilestuff
	msbuild "%QUICKSLINGPATH%\cef_binary\build\libcef_dll\libcef_dll_wrapper.vcxproj " /t:Build /p:Configuration=Release
	msbuild "%QUICKSLINGPATH%\cef_binary\build\libcef_dll\libcef_dll_wrapper.vcxproj " /t:Build /p:Configuration=Debug
:END