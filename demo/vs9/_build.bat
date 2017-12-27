@echo Build Time:
@date /T  
@time /T
@echo  ===========================start build======================================

devenv "demovs9.sln" /%1 %2 /project "ndclient\ndclient.vcproj"
@if %errorlevel% NEQ 0  goto ERROR

devenv "demovs9.sln" /%1 %2 /project "test_srv\test_srv.vcproj"
@if %errorlevel% NEQ 0  goto ERROR

devenv "demovs9.sln" /%1 %2 /project "test\test.vcproj"
@if %errorlevel% NEQ 0  goto ERROR


devenv "demovs9.sln" /%1 %2 /project "proxy\proxy.vcproj"
@if %errorlevel% NEQ 0  goto ERROR

devenv "demovs9.sln" /%1 %2 /project "tool\tool.vcproj"
@if %errorlevel% NEQ 0  goto ERROR

:ERROR
@set build_err=%errorlevel%
@echo ===========================build end==========================================

@date /T  
@time /T

@exit /B %build_err%
