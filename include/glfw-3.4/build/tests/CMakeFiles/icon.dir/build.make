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
include tests/CMakeFiles/icon.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tests/CMakeFiles/icon.dir/compiler_depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/icon.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/icon.dir/flags.make

tests/CMakeFiles/icon.dir/codegen:
.PHONY : tests/CMakeFiles/icon.dir/codegen

tests/CMakeFiles/icon.dir/icon.c.obj: tests/CMakeFiles/icon.dir/flags.make
tests/CMakeFiles/icon.dir/icon.c.obj: tests/CMakeFiles/icon.dir/includes_C.rsp
tests/CMakeFiles/icon.dir/icon.c.obj: C:/Users/barte/Desktop/VoltaFramework/include/glfw-3.4/tests/icon.c
tests/CMakeFiles/icon.dir/icon.c.obj: tests/CMakeFiles/icon.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object tests/CMakeFiles/icon.dir/icon.c.obj"
	cd /d C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests && C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT tests/CMakeFiles/icon.dir/icon.c.obj -MF CMakeFiles\icon.dir\icon.c.obj.d -o CMakeFiles\icon.dir\icon.c.obj -c C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\tests\icon.c

tests/CMakeFiles/icon.dir/icon.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/icon.dir/icon.c.i"
	cd /d C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests && C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\tests\icon.c > CMakeFiles\icon.dir\icon.c.i

tests/CMakeFiles/icon.dir/icon.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/icon.dir/icon.c.s"
	cd /d C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests && C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\tests\icon.c -o CMakeFiles\icon.dir\icon.c.s

# Object files for target icon
icon_OBJECTS = \
"CMakeFiles/icon.dir/icon.c.obj"

# External object files for target icon
icon_EXTERNAL_OBJECTS =

tests/icon.exe: tests/CMakeFiles/icon.dir/icon.c.obj
tests/icon.exe: tests/CMakeFiles/icon.dir/build.make
tests/icon.exe: src/libglfw3.a
tests/icon.exe: tests/CMakeFiles/icon.dir/linkLibs.rsp
tests/icon.exe: tests/CMakeFiles/icon.dir/objects1.rsp
tests/icon.exe: tests/CMakeFiles/icon.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable icon.exe"
	cd /d C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\icon.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/icon.dir/build: tests/icon.exe
.PHONY : tests/CMakeFiles/icon.dir/build

tests/CMakeFiles/icon.dir/clean:
	cd /d C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests && $(CMAKE_COMMAND) -P CMakeFiles\icon.dir\cmake_clean.cmake
.PHONY : tests/CMakeFiles/icon.dir/clean

tests/CMakeFiles/icon.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4 C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\tests C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests C:\Users\barte\Desktop\VoltaFramework\include\glfw-3.4\build\tests\CMakeFiles\icon.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : tests/CMakeFiles/icon.dir/depend

