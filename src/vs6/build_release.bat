call _build.bat release
if not %errorlevel%==0  goto build_error

:build_end
@echo Build SUCCESS!!
goto :EOF

:build_error
cd ..
@echo Build project error!
@Pause
