@set builder=devenv
call _build.bat build debug 
@if %errorlevel% NEQ 0  goto ERROR
@exit 

:ERROR 
@rem msg %username% "����������飡"
@echo buld project ~~~~~~~~~~~ error ~~~~~~~~~~~
@pause