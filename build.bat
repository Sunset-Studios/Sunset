@echo off
powershell -NoProfile -ExecutionPolicy Bypass -Command "& '%~dp0/engine/scripts/build.ps1'"
pause