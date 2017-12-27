@echo Build Time:
@date /T  
@time /T
@echo  ===========================start build======================================

devenv "nd_vc9.sln" /%1 %2 /project "nd_common\nd_common.vcproj"
@if %errorlevel% NEQ 0  goto ERROR


devenv "nd_vc9.sln" /%1 %2 /project "nd_crypt\nd_crypt.vcproj"
@if %errorlevel% NEQ 0  goto ERROR


devenv "nd_vc9.sln" /%1 %2 /project "nd_net\nd_net.vcproj"
@if %errorlevel% NEQ 0  goto ERROR


devenv "nd_vc9.sln" /%1 %2 /project "nd_srvcore\nd_srvcore.vcproj"
@if %errorlevel% NEQ 0  goto ERROR

devenv "nd_vc9.sln" /%1 %2 /project "nd_app\nd_app.vcproj"
@if %errorlevel% NEQ 0  goto ERROR

devenv "nd_vc9.sln" /%1 %2 /project "nd_appcpp\nd_appcpp.vcproj"
@if %errorlevel% NEQ 0  goto ERROR

devenv "nd_vc9.sln" /%1 %2 /project "nd_cliapp\nd_cliapp.vcproj"
@if %errorlevel% NEQ 0  goto ERROR

devenv "nd_vc9.sln" /%1 %2 /project "vs9_static_cli\vs9_static_cli.vcproj"
@if %errorlevel% NEQ 0  goto ERROR

:ERROR
@set build_err=%errorlevel%
@echo ===========================build end==========================================

@date /T  
@time /T

@exit /B %build_err%
