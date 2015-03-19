cmake_minimum_required(VERSION 3.0)

project(erijake)

# Compile the cppl executable
add_executable (ejasm src/asm.cpp)
add_executable (ejvm src/vm.cpp)
