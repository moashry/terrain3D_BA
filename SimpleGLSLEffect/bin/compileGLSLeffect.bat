REM this is supposed to be called from the Visual Studio environment
@cl /P /C "%~1"
@move /Y "%~n1.i" "%~dpn1.i"

REM store the absolute path of the file
@setlocal
@set target=%~dpn1.i
@cd /d "%~dp0"

@GFXcompiler --originalFilename "%~1" "%target%"
