
call _build.bat build debug 
@if %errorlevel% NEQ 0  goto ERROR
@exit 

:ERROR 
msg %username% "����������飡"
@echo buld project ~~~~~~~~~~~ error ~~~~~~~~~~~
@pause