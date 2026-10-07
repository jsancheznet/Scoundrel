echo off

if not exist build mkdir build
pushd build

set SDLInclude=-I"..\libraries\SDL-release-3.4.8\include"
set GLADInclude=-I"..\libraries\glad\include"
set GLMInclude=-I"..\libraries\glm-1.0.1-light"
set STBInclude=-I"..\libraries\stb"
set JSONInclude=-I"..\libraries\json"

set IncludeDirectories= %SDLInclude% %GLADInclude% %GLMInclude% %STBInclude% %JSONInclude%

echo.
echo COPYING ASSETS TO BUILD DIRECTORY...
xcopy /S /E /D /Y "..\assets" "assets\"
echo.

echo COPYING SHADERS TO BUILD DIRECTORY...
xcopy /S /E /D /Y "..\src\shaders" "shaders\"
echo.

echo.
echo BUILDING...
echo.

REM Debug build
clang-cl /Zi /Od -W4 -Wextra -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-unused-value^
	 ..\src\scoundrel.cpp ^
	 ..\src\log.cpp ^
	 ..\src\application.cpp ^
	 ..\src\renderer.cpp ^
	 ..\src\camera.cpp ^
	 ..\src\mouse.cpp ^
	 ..\src\keyboard.cpp ^
	 ..\src\audio.cpp ^
	 ..\src\asset_manager.cpp ^
	 ..\src\bitmap_font.cpp ^
	 ..\src\helpers.cpp ^
	 ..\libraries\glad\src\glad.c ^
	 %IncludeDirectories% ^
	 /Fe:scoundrel.exe ^
	 /link /LIBPATH:"..\libraries\SDL-release-3.4.8\Release" -SUBSYSTEM:CONSOLE SDL3.lib

if errorlevel 1 (
   popd
   exit /b 1
)

popd
