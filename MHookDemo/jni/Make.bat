@echo off
color 0A
set base_dir=%~dp0
%
base_dir:~0,2%
pushd %base_dir%
call D:\\PATH\\Android\\android-ndk-r10x64\\ndk-build.cmd