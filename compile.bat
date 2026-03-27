@echo off
powershell -ExecutionPolicy Bypass -File "%~dp0compile.ps1"
if %errorlevel% neq 0 pause
