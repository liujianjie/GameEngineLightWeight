@echo off
e:
cd E:\AllWorkSpace1\GameEngineLightWeight\scripts
pushd ..\
call vendor\bin\premake\premake5.exe vs2019
popd
PAUSE