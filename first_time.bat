set QUICKSLINGPATH=%cd%
if not exist "%cd%\cef_binary\CURRENT_VERSION_3.2526.1347.gcf20046" (
	if not exist "%cd%\archives" (
		mkdir archives
	)

	if not exist "archives\cef_binary_3.2526.1347.gcf20046_windows32.7z" ( 
		..\tools\wget https://storage.googleapis.com/quicksling/build/cef_binary_3.2526.1347.gcf20046_windows32.7z
	)
	"%QUICKSLINGPATH%\tools\7za.exe" x -y -o"%QUICKSLINGPATH%" "archives\cef_binary_3.2526.1347.gcf20046_windows32.7z" "cef_binary_3.2526.1347.gcf20046_windows32\Debug" "cef_binary_3.2526.1347.gcf20046_windows32\Release"
	robocopy "cef_binary_3.2526.1347.gcf20046_windows32\Debug" "%QUICKSLINGPATH%\cef_binary\Debug" /E /IS /MOVE
	robocopy "cef_binary_3.2526.1347.gcf20046_windows32\Release" "%QUICKSLINGPATH%\cef_binary\Release" /E /IS /MOVE
	rmdir /S /Q cef_binary_3.2526.1347.gcf20046_windows32

	if not exist "%cd%\cef_binary\build" (
		mkdir "%cd%\cef_binary\build"
		cd "%cd%\cef_binary\build"
		cmake -G "Visual Studio 12" ..
	)
	cd "%QUICKSLINGPATH%\cef_binary"
	type nul >>CURRENT_VERSION_3.2526.1347.gcf20046 & copy CURRENT_VERSION_3.2526.1347.gcf20046 +,,
	cd "%QUICKSLINGPATH%"
)
