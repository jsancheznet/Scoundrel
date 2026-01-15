@echo off

pushd build


set SDLInclude=-I"..\libraries\SDL3-3.2.10\include"
set GLADInclude=-I"..\libraries\glad\include"
set GLMInclude=-I"..\libraries\glm-1.0.1-light"
set STBInclude=-I"..\libraries\stb"
REM set JSONInclude=-I"..\libraries\json"

set IncludeDirectories= %SDLInclude% %GLADInclude% %GLMInclude% %STBInclude% %JSONInclude%

echo.
echo COPYING ASSETS TO BUILD DIRECTORY...
echo.

IF NOT EXIST "assets" MKDIR "assets"
COPY /Y "..\assets\*.*" "assets\"

IF NOT EXIST "shaders" MKDIR "shaders"
COPY /Y "..\shaders\*.*" "shaders\"

echo.
echo BUILDING...
echo.

REM This builds in debug mode for now
clang-cl /Zi /Od /WX ^
	 ..\src\scoundrel.cpp ^
	 ..\src\log.cpp ^
	 ..\src\application.cpp ^
	 ..\src\renderer.cpp ^
	 ..\src\camera.cpp ^
	 ..\libraries\glad\src\glad.c ^
	 %GLADInclude%  %IncludeDirectories% /link /LIBPATH:"..\libraries\SDL3-3.2.10\build\Debug" -SUBSYSTEM:CONSOLE SDL3.lib

popd
