call _build.bat build release 
@if %errorlevel% NEQ 0  goto ERROR
goto :EOF

:ERROR 
msg %username% "����������飡"
@echo buld project ~~~~~~~~~~~ error ~~~~~~~~~~~
@pause