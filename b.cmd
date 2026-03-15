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
g++ -O3 -march=native -flto main.cpp -o main.exe -lws2_32
if %errorlevel% equ 0 (
    echo Compilation successful! Output: main.exe
) else (
    echo Compilation failed!
)
goto end

:msvc
echo Compiling with cl...
cl /EHsc /O2 main.cpp /Fe:main.exe
if %errorlevel% equ 0 (
    echo Compilation successful! Output: main.exe
    if exist main.obj (
        echo Cleaning up main.obj...
        del main.obj
    )
) else (
    echo Compilation failed!
)
goto end

:end
pause