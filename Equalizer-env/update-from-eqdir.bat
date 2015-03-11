@echo > updateLog.txt

@call :copy-files Win32\Debug
@call :copy-files Win32\Release
@call :copy-files x64\Debug
@call :copy-files x64\Release

@goto :eof
 
:copy-files
@echo Copying %1 configuration from SVN build..
@mkdir %1 2>> updateLog.txt
@mkdir %1\Headers 2>> updateLog.txt
@xcopy %EQ_DIR%\%1\Headers %1\Headers /Y /E >> updateLog.txt
@copy %EQ_DIR%\%1\Equalizer.* %1\ /Y >> updateLog.txt
@copy %EQ_DIR%\%1\EqualizerServer.* %1\ /Y >> updateLog.txt
@copy %EQ_DIR%\%1\pthreadVC2.* %1\ /Y >> updateLog.txt
@goto :eof