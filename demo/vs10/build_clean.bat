rem @echo "Are you sure to cleanup ? (y|n)" 
rem @set /p choice= 
rem @if %choice% == n goto :eof

@set builder=devenv

@call _build.bat clean Debug

@call _build.bat clean release