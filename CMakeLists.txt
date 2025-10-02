
cmake_minimum_required(VERSION 3.10)
project(kzsh LANGUAGES C CXX)
set(KZSH_VERSION "0.1.1")
# Use local time for the build date
string(TIMESTAMP KSH_BUILD_DATE "%Y-%m-%d")
# Construct a target string from processor and system name
set(KZSH_TARGET "${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}")
add_executable(kzsh
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
target_compile_definitions(kzsh PRIVATE \
	KZSH_VERSION="${KZSH_VERSION}" \
	KZSH_BUILD_DATE="${KZSH_BUILD_DATE}" \
	KZSH_TARGET="${KZSH_TARGET}" 
)
