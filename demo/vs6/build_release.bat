

call  _build_demo.bat release
if not %errorlevel%==0  goto build_error

:build_end
@echo Build SUCCESS!!
@exit 

:build_error
cd ..
@echo Build project error!
@Pause
@exit 
