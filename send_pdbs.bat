set QUICKSLINGPATH=%~dp0

if not exist "%QUICKSLINGPATH%\Release\BugSplat.pdb" (
	copy "%QUICKSLINGPATH%\BugSplat\bin\BugSplat.pdb" "%QUICKSLINGPATH%\Release\" 
)

if not exist "%QUICKSLINGPATH%\Release\BugSplat.dll" (
	copy "%QUICKSLINGPATH%\BugSplat\bin\BugSplat.dll" "%QUICKSLINGPATH%\Release\" 
)

if not exist "%QUICKSLINGPATH%\Release\libcef.dll" (
	copy "%QUICKSLINGPATH%\cef_binary\Release\libcef.dll" "%QUICKSLINGPATH%\Release\" 
)

BugSplat\bin\SendPdbs /u info+quickslingshell@quicklet.io /p R3dC4r /a QuickslingShell /l Release\QuickslingShell.exe /b QuickslingShell1_1 /d Release /f "QuickslingShell.pdb;QuickslingShell.exe;BugSplat.pdb;BugSplat.dll"
BugSplat\bin\SendPdbs /u info+quicksling@quicklet.io /p R3dC4r /a Quicksling /l Release\Quicksling.exe /b Quicksling1_0 /d Release /f "Quicksling.pdb;Quicksling.exe;BugSplat.pdb;BugSplat.dll"
