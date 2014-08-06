
call _build.bat build debug 
@if %errorlevel% NEQ 0  goto ERROR
goto :EOF

:ERROR 
msg %username% "±‡“Î¥ÌŒÛ£¨«ÎºÏ≤È£°"
@echo buld project ~~~~~~~~~~~ error ~~~~~~~~~~~
@pause