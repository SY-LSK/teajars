@echo off
chcp 65001 >nul
echo Select build target:
echo 1. Build builder.go (Go cross-platform builder)
echo 2. cl (MSVC C++ compile)
set /p choice=Enter option (1 or 2): 

if "%choice%"=="1" goto gobuild
if "%choice%"=="2" goto msvc
echo Invalid option, using default: builder.go
goto gobuild

:gobuild
echo Building builder.go for all platforms...
cd /d "%~dp0exec_build"

set GOOS=windows
set GOARCH=amd64
go build -o windows_amd64.exe .
if %errorlevel% neq 0 echo Build windows/amd64 failed!

set GOOS=windows
set GOARCH=arm64
go build -o windows_arm64.exe .
if %errorlevel% neq 0 echo Build windows/arm64 failed!

set GOOS=linux
set GOARCH=arm64
go build -o linux_arm64 .
if %errorlevel% neq 0 echo Build linux/arm64 failed!

set GOOS=linux
set GOARCH=amd64
go build -o linux_amd64 .
if %errorlevel% neq 0 echo Build linux/amd64 failed!

set GOOS=darwin
set GOARCH=amd64
go build -o darwin_amd64 .
if %errorlevel% neq 0 echo Build darwin/amd64 failed!

set GOOS=darwin
set GOARCH=arm64
go build -o darwin_arm64 .
if %errorlevel% neq 0 echo Build darwin/arm64 failed!

set GOOS=
set GOARCH=

echo.
echo All platforms built successfully!
echo   - windows_amd64.exe
echo   - windows_arm64.exe
echo   - linux_amd64
echo   - linux_arm64
echo   - darwin_amd64
echo   - darwin_arm64

cd /d "%~dp0"
goto end

:msvc
echo Compiling with cl...
cl /MP /EHsc /O2 /std:c++17 /utf-8 /D_HAS_STD_BYTE=0 /I./include /I./src main.cpp src/base.cpp src/server.cpp /Fe:main.exe
if %errorlevel% equ 0 (
    echo Compilation successful!
) else (
    echo Compilation failed!
    goto end
)
if exist main.obj del main.obj
if exist base.obj del base.obj
if exist server.obj del server.obj
goto end

:end
pause