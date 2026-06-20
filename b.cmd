@echo off
chcp 65001 >nul
echo Select compiler:
echo 1. g++ (MinGW)
echo 2. cl (MSVC)
set /p choice=Enter option (1 or 2): 

if "%choice%"=="1" goto gcc
if "%choice%"=="2" goto msvc
echo Invalid option, using default compiler g++
goto gcc

:gcc
echo Compiling with g++...
g++ -O3 -march=native -std=c++17 -I./include -I./src -flto main.cpp src/base.cpp src/server.cpp -o main.exe -lws2_32
if %errorlevel% equ 0 (
    echo Compilation successful! Output: main.exe
) else (
    echo Compilation failed!
)
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