@echo off

mkdir ..\build
pushd ..\build
cl /ZI ..\code\main.c /I "..\code\include" "..\code\lib\SDL3.lib" "..\code\lib\SDL3_ttf.lib"
popd
