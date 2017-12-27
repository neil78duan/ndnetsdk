@echo Build Time:
@date /T  
@time /T
@echo  ===========================start build======================================


%builder% "vs10.sln" /%1 %2 /project "nd_common\nd_common.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR


%builder% "vs10.sln" /%1 %2 /project "nd_crypt\nd_crypt.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR


%builder% "vs10.sln" /%1 %2 /project "nd_net\nd_net.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR


%builder% "vs10.sln" /%1 %2 /project "nd_srvcore\nd_srvcore.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR


%builder% "vs10.sln" /%1 %2 /project "srv_libs\srv_libs.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR

%builder% "vs10.sln" /%1 %2 /project "vs10_static_cli\vs10_static_cli.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR

%builder% "vs10.sln" /%1 %2 /project "nd_app\nd_app.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR

%builder% "vs10.sln" /%1 %2 /project "nd_appcpp\nd_appcpp.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR

%builder% "vs10.sln" /%1 %2 /project "nd_cliapp\nd_cliapp.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR

:ERROR
@set build_err=%errorlevel%
@echo ===========================build end==========================================

@date /T  
@time /T

@exit /B %build_err%
