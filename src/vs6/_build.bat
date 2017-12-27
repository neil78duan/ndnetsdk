@echo off
if "%1" == "" goto :build_nothing
if "%1" == "debug" goto :build_debug
if "%1" == "release" goto :build_release
if "%1" == "unicode" goto :build_unicode
if "%1" == "clean" goto :build_clean

rem============================================================================
:build_nothing
goto :build_end

:build_release

rem============================================================================

msdev nd_common/nd_common.dsp /make "nd_common - Win32 Release"
if not %errorlevel%==0  goto build_error

msdev nd_crypt/nd_crypt.dsp /make "nd_crypt - Win32 Release"
if not %errorlevel%==0  goto build_error

msdev nd_net/nd_net.dsp /make "nd_net - Win32 Release"
if not %errorlevel%==0  goto build_error

msdev nd_srvcore/nd_srvcore.dsp /make "nd_srvcore - Win32 Release"
if not %errorlevel%==0  goto build_error

msdev nd_app/nd_app.dsp /make "nd_app - Win32 Release"
if not %errorlevel%==0  goto build_error

msdev nd_appcpp/nd_appcpp.dsp /make "nd_appcpp - Win32 Release"
if not %errorlevel%==0  goto build_error

msdev nd_cliapp/nd_cliapp.dsp /make "nd_cliapp - Win32 Release"
if not %errorlevel%==0  goto build_error

goto :build_end

REM ------------------------------------------------------------------
REM 
REM ------------------------------------------------------------------

:build_debug

msdev nd_common/nd_common.dsp /make "nd_common - Win32 Debug"
if not %errorlevel%==0  goto build_error

msdev nd_crypt/nd_crypt.dsp /make "nd_crypt - Win32 Debug"
if not %errorlevel%==0  goto build_error

msdev nd_net/nd_net.dsp /make "nd_net - Win32 Debug"
if not %errorlevel%==0  goto build_error

msdev nd_srvcore/nd_srvcore.dsp /make "nd_srvcore - Win32 Debug"
if not %errorlevel%==0  goto build_error

msdev nd_app/nd_app.dsp /make "nd_app - Win32 Debug"
if not %errorlevel%==0  goto build_error

msdev nd_appcpp/nd_appcpp.dsp /make "nd_appcpp - Win32 Debug"
if not %errorlevel%==0  goto build_error

msdev nd_cliapp/nd_cliapp.dsp /make "nd_cliapp - Win32 Debug"
if not %errorlevel%==0  goto build_error

goto :build_end

rem =====================================================================
rem =====================================================================
:build_clean

msdev nd_common/nd_common.dsp /make  ALL /clean
if not %errorlevel%==0  goto build_error

msdev nd_crypt/nd_crypt.dsp /make  ALL /clean
if not %errorlevel%==0  goto build_error

msdev nd_net/nd_net.dsp /make  ALL /clean
if not %errorlevel%==0  goto build_error

msdev nd_srvcore/nd_srvcore.dsp /make  ALL /clean
if not %errorlevel%==0  goto build_error

msdev nd_app/nd_app.dsp /make  ALL /clean
if not %errorlevel%==0  goto build_error

msdev nd_appcpp/nd_appcpp.dsp /make ALL /clean
if not %errorlevel%==0  goto build_error

msdev nd_cliapp/nd_cliapp.dsp /make  ALL /clean
if not %errorlevel%==0  goto build_error

goto :build_end

:build_end
@echo Build SUCCESS!!
@exit /B 0

:build_error
@exit /B 1
