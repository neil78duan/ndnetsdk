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

msdev ndclient/ndclient.dsp /make "ndclient - Win32 Release"
if not %errorlevel%==0  goto build_error

msdev test_srv/test_srv.dsp /make "test_srv - Win32 Release"
if not %errorlevel%==0  goto build_error

msdev test/test.dsp /make "test - Win32 Release"
if not %errorlevel%==0  goto build_error

goto :build_end

REM ------------------------------------------------------------------
REM 
REM ------------------------------------------------------------------

:build_debug

msdev ndclient/ndclient.dsp /make "ndclient - Win32 Debug"
if not %errorlevel%==0  goto build_error

msdev test_srv/test_srv.dsp /make "test_srv - Win32 Debug"
if not %errorlevel%==0  goto build_error

msdev test/test.dsp /make "test - Win32 Debug"
if not %errorlevel%==0  goto build_error

goto :build_end

rem =====================================================================
rem =====================================================================
:build_clean

msdev ndclient/ndclient.dsp /make  ALL /clean
if not %errorlevel%==0  goto build_error

msdev test_srv/test_srv.dsp /make   ALL /clean
if not %errorlevel%==0  goto build_error

msdev test/test.dsp /make  ALL /clean
if not %errorlevel%==0  goto build_error

goto :build_end

:build_end
@echo Build SUCCESS!!
@exit /B %errorlevel%

:build_error
set el=%errorlevel%
@echo Build project error!
@echo errorlevel=%errorlevel%

@exit /B %el%
