
cmake_minimum_required(VERSION 3.10)
project(ksh LANGUAGES C CXX)
set(KSH_VERSION "0.1")
# Use local time for the build date
string(TIMESTAMP KSH_BUILD_DATE "%Y-%m-%d")
# Construct a target string from processor and system name
set(KSH_TARGET "${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}")
add_executable(ksh
	src/main.c
	src/builtins.c
	src/builtins.cpp
	src/builtins_cpp.cpp
	src/shell.c
	src/exec.c
	src/utils.c
	src/history.c
	src/env.c
	src/alias.c
)
target_compile_definitions(ksh PRIVATE \
	KSH_VERSION="${KSH_VERSION}" \
	KSH_BUILD_DATE="${KSH_BUILD_DATE}" \
	KSH_TARGET="${KSH_TARGET}" 
)
