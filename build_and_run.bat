@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=amd64
cd /d C:\PAVIC_LAB_2025
rmdir /s /q build_cuda 2>nul
cmake -S . -B build_cuda -G "Visual Studio 17 2022" -A x64 -DOpenCV_DIR=C:\opencv\build
cmake --build build_cuda --config Release
