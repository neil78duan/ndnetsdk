@echo Build Time:
@date /T  
@time /T
@echo  ===========================start build======================================

%builder% "vs10.sln" /%1 %2 /project "ndclient\ndclient.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR

%builder% "vs10.sln" /%1 %2 /project "test_srv\test_srv.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR



%builder% "vs10.sln" /%1 %2 /project "test\test.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR

%builder% "vs10.sln" /%1 %2 /project "tool\tool.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR

%builder% "vs10.sln" /%1 %2 /project "proxy\proxy.vcxproj"
@if %errorlevel% NEQ 0  goto ERROR


:ERROR
@set build_err=%errorlevel%
@echo ===========================build end==========================================

@date /T  
@time /T

@exit /B %build_err%
