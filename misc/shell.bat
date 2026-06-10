@echo off
call "D:\VisualStudio\VC\Auxiliary\Build\vcvarsall.bat" x64 8.1
set INCLUDE=C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\winrt;%INCLUDE%
set path=s:\misc;%path%

cd /D s:\misc
