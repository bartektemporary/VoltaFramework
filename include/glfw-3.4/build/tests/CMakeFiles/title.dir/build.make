# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.31

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build

# Include any dependencies generated for this target.
include tests/CMakeFiles/title.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tests/CMakeFiles/title.dir/compiler_depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/title.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/title.dir/flags.make

tests/CMakeFiles/title.dir/codegen:
.PHONY : tests/CMakeFiles/title.dir/codegen

tests/CMakeFiles/title.dir/title.c.obj: tests/CMakeFiles/title.dir/flags.make
tests/CMakeFiles/title.dir/title.c.obj: tests/CMakeFiles/title.dir/includes_C.rsp
tests/CMakeFiles/title.dir/title.c.obj: C:/Users/barte/Desktop/VoltaFramework/include/glfw-3.4/tests/title.c
tests/CMakeFiles/title.dir/title.c.obj: tests/CMakeFiles/title.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object tests/CMakeFiles/title.dir/title.c.obj"
	cd /d C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests && C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT tests/CMakeFiles/title.dir/title.c.obj -MF CMakeFiles\title.dir\title.c.obj.d -o CMakeFiles\title.dir\title.c.obj -c C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\tests\title.c

tests/CMakeFiles/title.dir/title.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/title.dir/title.c.i"
	cd /d C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests && C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\tests\title.c > CMakeFiles\title.dir\title.c.i

tests/CMakeFiles/title.dir/title.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/title.dir/title.c.s"
	cd /d C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests && C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\tests\title.c -o CMakeFiles\title.dir\title.c.s

# Object files for target title
title_OBJECTS = \
"CMakeFiles/title.dir/title.c.obj"

# External object files for target title
title_EXTERNAL_OBJECTS =

tests/title.exe: tests/CMakeFiles/title.dir/title.c.obj
tests/title.exe: tests/CMakeFiles/title.dir/build.make
tests/title.exe: src/libglfw3.a
tests/title.exe: tests/CMakeFiles/title.dir/linkLibs.rsp
tests/title.exe: tests/CMakeFiles/title.dir/objects1.rsp
tests/title.exe: tests/CMakeFiles/title.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable title.exe"
	cd /d C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\title.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/title.dir/build: tests/title.exe
.PHONY : tests/CMakeFiles/title.dir/build

tests/CMakeFiles/title.dir/clean:
	cd /d C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests && $(CMAKE_COMMAND) -P CMakeFiles\title.dir\cmake_clean.cmake
.PHONY : tests/CMakeFiles/title.dir/clean

tests/CMakeFiles/title.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4 C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\tests C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests\CMakeFiles\title.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : tests/CMakeFiles/title.dir/depend

