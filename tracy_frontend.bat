@echo off
call "./engine/third_party/tracy/vcpkg/install_vcpkg_dependencies.bat"
devenv.com "./engine/third_party/tracy/profiler/build/win32/Tracy.sln" /build Release
"./engine/third_party/tracy/profiler/build/win32/x64/Release/Tracy.exe"