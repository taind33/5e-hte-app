@echo off
setlocal EnableExtensions
rem ======================================================================
rem  deploy.bat - Dong goi ung dung Qt 6 + CMake tren Windows (dung chung)
rem
rem  Tu nhan dien: Qt kit (MinGW / llvm-mingw / MSVC), compiler, generator,
rem                ten file .exe, cac thu muc QML.
rem
rem  Cach dung:
rem    - Copy file nay vao thu muc goc du an (canh CMakeLists.txt), double-click
rem    - Hoac goi tu noi khac:   deploy.bat "D:\duong\dan\du_an"
rem
rem  Ket qua:  <du_an>\dist\<TenApp>\   va   <du_an>\dist\<TenApp>.zip
rem
rem  Ghi de cau hinh (TUY CHON): tao file deploy.config.bat trong du an,
rem  chi can nhung dong muon doi, vi du:
rem      set "QT_DIR=C:\Qt\6.11.2\mingw_64"      & rem chon Qt kit cu the
rem      set "APP_EXE=5E-HTE.exe"                 & rem khi build ra nhieu .exe
rem      set "PACKAGE_NAME=5E-HTE_v1.0"           & rem ten thu muc/zip dau ra
rem      set "QML_DIRS=qml ui"                    & rem hoac "none" (app Widgets)
rem      set "BUILD_TYPE=Release"                 & rem Release/RelWithDebInfo/Debug
rem      set "EXTRA_DIR=third_party\bin"          & rem file kem them (mac dinh deploy_extra)
rem      set "WINDEPLOYQT_EXTRA=--no-translations --no-system-d3d-compiler"
rem      set "CMAKE_EXTRA_ARGS=-DMY_OPTION=ON"
rem      set "MAKE_ZIP=0"
rem      set "QT_BASE=D:\Qt"                      & rem noi cai Qt (mac dinh C:\Qt)
rem  Dat bien NO_PAUSE=1 de chay khong dung lai (CI / script khac goi).
rem ======================================================================

rem ---------- 1. Thu muc du an ----------
if "%~1"=="" (set "SRC=%~dp0") else (set "SRC=%~f1")
if "%SRC:~-1%"=="\" set "SRC=%SRC:~0,-1%"
if not exist "%SRC%\CMakeLists.txt" (
    echo [LOI] Khong thay CMakeLists.txt trong "%SRC%"
    goto :fail
)
pushd "%SRC%"

rem ---------- 2. Cau hinh mac dinh + file cau hinh rieng ----------
set "QT_BASE=C:\Qt"
set "QT_DIR="
set "APP_EXE="
set "PACKAGE_NAME="
set "QML_DIRS="
set "BUILD_TYPE=Release"
set "BUILD="
set "EXTRA_DIR=deploy_extra"
set "WINDEPLOYQT_EXTRA=--no-translations"
set "CMAKE_EXTRA_ARGS="
set "MAKE_ZIP=1"
if exist "%SRC%\deploy.config.bat" (
    echo Doc cau hinh: deploy.config.bat
    call "%SRC%\deploy.config.bat"
)
if not defined BUILD set "BUILD=%SRC%\build\%BUILD_TYPE%"
for %%i in ("%BUILD%") do set "BUILD=%%~fi"
for %%i in ("%EXTRA_DIR%") do set "EXTRA_DIR=%%~fi"
set "DIST_ROOT=%SRC%\dist"

rem ---------- 3. Tim Qt kit ----------
set "CXX_FROM_CACHE="
if not defined QT_DIR call :detect_from_cache
if not defined QT_DIR call :detect_latest_qt
if not defined QT_DIR goto :no_qt
if not exist "%QT_DIR%\bin\windeployqt.exe" goto :no_qt
for %%i in ("%QT_DIR%\..\..") do set "QT_ROOT=%%~fi"
for %%i in ("%QT_DIR%") do set "KIT=%%~nxi"

set "TOOLCHAIN=mingw"
if /i not "%KIT:msvc=%"=="%KIT%" set "TOOLCHAIN=msvc"
if /i not "%KIT:llvm=%"=="%KIT%" set "TOOLCHAIN=llvm-mingw"

set "PATH=%QT_DIR%\bin;%PATH%"
if exist "%QT_ROOT%\Tools\CMake_64\bin\cmake.exe" set "PATH=%QT_ROOT%\Tools\CMake_64\bin;%PATH%"
if exist "%QT_ROOT%\Tools\Ninja\ninja.exe" set "PATH=%QT_ROOT%\Tools\Ninja;%PATH%"

rem ---------- 4. Compiler ----------
set "CXX_ARG="
if "%TOOLCHAIN%"=="msvc" goto :env_msvc

set "CXX_NAME=g++.exe"
set "TC_PAT=mingw*_64"
if "%TOOLCHAIN%"=="llvm-mingw" set "CXX_NAME=clang++.exe"
if "%TOOLCHAIN%"=="llvm-mingw" set "TC_PAT=llvm-mingw*_64"
set "MINGW_BIN="
if defined CXX_FROM_CACHE for %%i in ("%CXX_FROM_CACHE:/=\%") do set "MINGW_BIN=%%~dpi"
if defined MINGW_BIN set "MINGW_BIN=%MINGW_BIN:~0,-1%"
if defined MINGW_BIN if not exist "%MINGW_BIN%\%CXX_NAME%" set "MINGW_BIN="
if not defined MINGW_BIN for /d %%d in ("%QT_ROOT%\Tools\%TC_PAT%") do if exist "%%d\bin\%CXX_NAME%" set "MINGW_BIN=%%d\bin"
if not defined MINGW_BIN goto :no_compiler
set "PATH=%MINGW_BIN%;%PATH%"
set CXX_ARG=-DCMAKE_CXX_COMPILER="%MINGW_BIN:\=/%/%CXX_NAME%"
goto :env_done

:env_msvc
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :no_compiler
set "VSDIR="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
if not defined VSDIR goto :no_compiler
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul || goto :no_compiler

:env_done
where cmake >nul 2>&1 || goto :no_cmake

rem ---------- 5. Generator (giu generator cu neu thu muc build da ton tai) ----------
set "GEN="
if exist "%BUILD%\CMakeCache.txt" for /f "tokens=1,* delims==" %%a in ('findstr /b /c:"CMAKE_GENERATOR:INTERNAL=" "%BUILD%\CMakeCache.txt"') do set "GEN=%%b"
if defined GEN goto :gen_done
where ninja >nul 2>&1
if not errorlevel 1 set "GEN=Ninja"
if not defined GEN if "%TOOLCHAIN%"=="msvc" set "GEN=NMake Makefiles"
if not defined GEN set "GEN=MinGW Makefiles"
:gen_done

echo.
echo ================================================================
echo  Du an    : %SRC%
echo  Qt kit   : %QT_DIR%
echo  Compiler : %TOOLCHAIN%
echo  Generator: %GEN%
echo  Build    : %BUILD%  [%BUILD_TYPE%]
echo ================================================================
echo.

rem ---------- 6. Configure + Build ----------
echo [1/4] Configure...
cmake -S "%SRC%" -B "%BUILD%" -G "%GEN%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_PREFIX_PATH="%QT_DIR:\=/%" %CXX_ARG% %CMAKE_EXTRA_ARGS% || goto :fail
echo [2/4] Build...
cmake --build "%BUILD%" --parallel || goto :fail

rem ---------- 7. Tim file .exe ----------
echo [3/4] Deploy...
set "EXE_PATH="
set "EXE_COUNT=0"
set "EXE_FILTER=*.exe"
if defined APP_EXE set "EXE_FILTER=%APP_EXE%"
for /f "delims=" %%f in ('dir /b /s /a-d "%BUILD%\%EXE_FILTER%" 2^>nul ^| findstr /i /v /c:"CMakeFiles" ^| findstr /i /v /c:"_deps"') do call :count_exe "%%f"
if %EXE_COUNT%==0 (
    echo [LOI] Khong tim thay file .exe [%EXE_FILTER%] trong "%BUILD%"
    goto :fail
)
if %EXE_COUNT% GTR 1 (
    echo [LOI] Tim thay %EXE_COUNT% file .exe - dat APP_EXE trong deploy.config.bat
    goto :fail
)
for %%i in ("%EXE_PATH%") do set "APP_EXE=%%~nxi"
for %%i in ("%EXE_PATH%") do set "EXE_NAME=%%~ni"
for %%i in ("%EXE_PATH%") do set "EXE_DIR=%%~dpi"
if not defined PACKAGE_NAME set "PACKAGE_NAME=%EXE_NAME%"
set "DIST=%DIST_ROOT%\%PACKAGE_NAME%"

rem ---------- 8. Thu muc QML can quet ----------
set "QML_ARGS="
if /i "%QML_DIRS%"=="none" goto :qml_done
if defined QML_DIRS goto :qml_from_config
if exist "%SRC%\*.qml" call :add_root_qml
for /d %%d in ("%SRC%\*") do call :add_qml_dir "%%d"
goto :qml_done
:qml_from_config
for %%d in (%QML_DIRS%) do call :add_qml_arg "%%~fd"
:qml_done
if defined QML_ARGS (echo     QML:%QML_ARGS%) else (echo     QML: khong co - app Widgets)

rem ---------- 9. Copy exe + windeployqt + file kem ----------
set "DEPLOY_MODE=--release"
if /i "%BUILD_TYPE%"=="Debug" set "DEPLOY_MODE=--debug"
if exist "%DIST%" rmdir /s /q "%DIST%"
mkdir "%DIST%" || goto :fail
copy /y "%EXE_PATH%" "%DIST%\" >nul || goto :fail
rem DLL do chinh du an build ra (add_library SHARED) nam canh exe
if exist "%EXE_DIR%*.dll" copy /y "%EXE_DIR%*.dll" "%DIST%\" >nul
windeployqt %DEPLOY_MODE% --compiler-runtime %WINDEPLOYQT_EXTRA% %QML_ARGS% "%DIST%\%APP_EXE%" || goto :fail
if exist "%EXTRA_DIR%\" (
    echo     Kem them: %EXTRA_DIR%
    xcopy /y /e /i /q "%EXTRA_DIR%\*" "%DIST%\" >nul
)

rem ---------- 10. Nen zip ----------
if not "%MAKE_ZIP%"=="1" goto :done
echo [4/4] Zip...
set "ZIPFILE=%DIST_ROOT%\%PACKAGE_NAME%.zip"
if exist "%ZIPFILE%" del /q "%ZIPFILE%"
powershell -NoProfile -Command "Compress-Archive -Path '%DIST%' -DestinationPath '%ZIPFILE%' -Force" || goto :fail

:done
echo.
echo ================================================================
echo  XONG
echo  Thu muc : %DIST%
if "%MAKE_ZIP%"=="1" echo  Zip     : %ZIPFILE%
echo ================================================================
popd
if not defined NO_PAUSE pause
exit /b 0

:no_qt
echo [LOI] Khong tim thay Qt kit. Dat QT_DIR trong deploy.config.bat
echo       vi du: set "QT_DIR=C:\Qt\6.11.2\mingw_64"
goto :fail
:no_compiler
echo [LOI] Khong tim thay compiler cho kit "%KIT%" [%TOOLCHAIN%].
echo       MinGW: cai qua Qt Maintenance Tool. MSVC: cai Visual Studio + C++ workload.
goto :fail
:no_cmake
echo [LOI] Khong tim thay cmake. Cai CMake qua Qt Maintenance Tool.
goto :fail

:fail
echo.
echo === LOI - xem thong bao o tren ===
popd 2>nul
if not defined NO_PAUSE pause
exit /b 1

rem ======================================================================
rem  Ham phu
rem ======================================================================

:detect_from_cache
rem Lay Qt kit + compiler tu thu muc build cua Qt Creator (build\*\CMakeCache.txt)
for /d %%d in ("%SRC%\build\*") do if not defined QT_DIR if exist "%%d\CMakeCache.txt" call :read_cache "%%d\CMakeCache.txt"
exit /b 0

:read_cache
set "QT6CFG="
set "CXX_FROM_CACHE="
for /f "tokens=1,* delims==" %%a in ('findstr /b /c:"Qt6_DIR:PATH=" "%~1"') do set "QT6CFG=%%b"
for /f "tokens=1,* delims==" %%a in ('findstr /b /c:"CMAKE_CXX_COMPILER:" "%~1"') do set "CXX_FROM_CACHE=%%b"
if not defined QT6CFG exit /b 0
set "QT6CFG=%QT6CFG:/=\%"
set "QT_DIR=%QT6CFG:\lib\cmake\Qt6=%"
if not exist "%QT_DIR%\bin\windeployqt.exe" set "QT_DIR="
exit /b 0

:detect_latest_qt
rem Chon Qt 6 moi nhat trong QT_BASE; uu tien mingw > msvc > llvm-mingw
if not exist "%QT_BASE%\" exit /b 0
for /f "usebackq delims=" %%q in (`powershell -NoProfile -Command "foreach ($d in (Get-ChildItem -LiteralPath '%QT_BASE%' -Directory -ErrorAction SilentlyContinue | Where-Object { $_.Name -match '\A6\.\d+\.\d+\z' } | Sort-Object { [version]$_.Name } -Descending)) { $k = Get-ChildItem -LiteralPath $d.FullName -Directory | Where-Object { $_.Name -match 'mingw|msvc' -and $_.Name -notmatch 'arm' -and (Test-Path (Join-Path $_.FullName 'bin\windeployqt.exe')) } | Sort-Object { if ($_.Name -like 'mingw*') {0} elseif ($_.Name -like 'msvc*') {1} else {2} } | Select-Object -First 1; if ($k) { $k.FullName; break } }"`) do set "QT_DIR=%%q"
exit /b 0

:count_exe
set /a EXE_COUNT+=1
set "EXE_PATH=%~1"
echo     Exe: %~1
exit /b 0

:add_root_qml
rem File .qml o thu muc goc (vd Splash.qml): copy ra cho rieng de quet,
rem tranh quet ca thu muc build/dist.
if exist "%BUILD%\_root_qml" rmdir /s /q "%BUILD%\_root_qml"
mkdir "%BUILD%\_root_qml"
copy /y "%SRC%\*.qml" "%BUILD%\_root_qml\" >nul
call :add_qml_arg "%BUILD%\_root_qml"
exit /b 0

:add_qml_dir
set "DN=%~nx1"
if "%DN:~0,1%"=="." exit /b 0
if /i "%DN:~0,5%"=="build" exit /b 0
if /i "%DN%"=="dist" exit /b 0
if /i "%DN%"=="out" exit /b 0
dir /s /b "%~1\*.qml" >nul 2>&1 || exit /b 0
call :add_qml_arg "%~1"
exit /b 0

:add_qml_arg
set QML_ARGS=%QML_ARGS% --qmldir "%~1"
exit /b 0
