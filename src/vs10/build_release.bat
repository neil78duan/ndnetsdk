@set builder=devenv

call _build.bat build release 
@if %errorlevel% NEQ 0  goto ERROR
goto :EOF

:ERROR 
@rem msg %username% "����������飡"
@echo buld project ~~~~~~~~~~~ error ~~~~~~~~~~~
@pause