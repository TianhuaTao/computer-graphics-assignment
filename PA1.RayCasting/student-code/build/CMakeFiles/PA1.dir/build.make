# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.2

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.2.3/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.2.3/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/sam/dev/graphics/PA1.RayCasting/student-code

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/sam/dev/graphics/PA1.RayCasting/student-code/build

# Include any dependencies generated for this target.
include CMakeFiles/PA1.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/PA1.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/PA1.dir/flags.make

CMakeFiles/PA1.dir/src/image.cpp.o: CMakeFiles/PA1.dir/flags.make
CMakeFiles/PA1.dir/src/image.cpp.o: ../src/image.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/sam/dev/graphics/PA1.RayCasting/student-code/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/PA1.dir/src/image.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/PA1.dir/src/image.cpp.o -c /Users/sam/dev/graphics/PA1.RayCasting/student-code/src/image.cpp

CMakeFiles/PA1.dir/src/image.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/PA1.dir/src/image.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /Users/sam/dev/graphics/PA1.RayCasting/student-code/src/image.cpp > CMakeFiles/PA1.dir/src/image.cpp.i

CMakeFiles/PA1.dir/src/image.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/PA1.dir/src/image.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /Users/sam/dev/graphics/PA1.RayCasting/student-code/src/image.cpp -o CMakeFiles/PA1.dir/src/image.cpp.s

CMakeFiles/PA1.dir/src/image.cpp.o.requires:
.PHONY : CMakeFiles/PA1.dir/src/image.cpp.o.requires

CMakeFiles/PA1.dir/src/image.cpp.o.provides: CMakeFiles/PA1.dir/src/image.cpp.o.requires
	$(MAKE) -f CMakeFiles/PA1.dir/build.make CMakeFiles/PA1.dir/src/image.cpp.o.provides.build
.PHONY : CMakeFiles/PA1.dir/src/image.cpp.o.provides

CMakeFiles/PA1.dir/src/image.cpp.o.provides.build: CMakeFiles/PA1.dir/src/image.cpp.o

CMakeFiles/PA1.dir/src/main.cpp.o: CMakeFiles/PA1.dir/flags.make
CMakeFiles/PA1.dir/src/main.cpp.o: ../src/main.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/sam/dev/graphics/PA1.RayCasting/student-code/build/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/PA1.dir/src/main.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/PA1.dir/src/main.cpp.o -c /Users/sam/dev/graphics/PA1.RayCasting/student-code/src/main.cpp

CMakeFiles/PA1.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/PA1.dir/src/main.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /Users/sam/dev/graphics/PA1.RayCasting/student-code/src/main.cpp > CMakeFiles/PA1.dir/src/main.cpp.i

CMakeFiles/PA1.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/PA1.dir/src/main.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /Users/sam/dev/graphics/PA1.RayCasting/student-code/src/main.cpp -o CMakeFiles/PA1.dir/src/main.cpp.s

CMakeFiles/PA1.dir/src/main.cpp.o.requires:
.PHONY : CMakeFiles/PA1.dir/src/main.cpp.o.requires

CMakeFiles/PA1.dir/src/main.cpp.o.provides: CMakeFiles/PA1.dir/src/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/PA1.dir/build.make CMakeFiles/PA1.dir/src/main.cpp.o.provides.build
.PHONY : CMakeFiles/PA1.dir/src/main.cpp.o.provides

CMakeFiles/PA1.dir/src/main.cpp.o.provides.build: CMakeFiles/PA1.dir/src/main.cpp.o

CMakeFiles/PA1.dir/src/mesh.cpp.o: CMakeFiles/PA1.dir/flags.make
CMakeFiles/PA1.dir/src/mesh.cpp.o: ../src/mesh.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/sam/dev/graphics/PA1.RayCasting/student-code/build/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/PA1.dir/src/mesh.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/PA1.dir/src/mesh.cpp.o -c /Users/sam/dev/graphics/PA1.RayCasting/student-code/src/mesh.cpp

CMakeFiles/PA1.dir/src/mesh.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/PA1.dir/src/mesh.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /Users/sam/dev/graphics/PA1.RayCasting/student-code/src/mesh.cpp > CMakeFiles/PA1.dir/src/mesh.cpp.i

CMakeFiles/PA1.dir/src/mesh.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/PA1.dir/src/mesh.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /Users/sam/dev/graphics/PA1.RayCasting/student-code/src/mesh.cpp -o CMakeFiles/PA1.dir/src/mesh.cpp.s

CMakeFiles/PA1.dir/src/mesh.cpp.o.requires:
.PHONY : CMakeFiles/PA1.dir/src/mesh.cpp.o.requires

CMakeFiles/PA1.dir/src/mesh.cpp.o.provides: CMakeFiles/PA1.dir/src/mesh.cpp.o.requires
	$(MAKE) -f CMakeFiles/PA1.dir/build.make CMakeFiles/PA1.dir/src/mesh.cpp.o.provides.build
.PHONY : CMakeFiles/PA1.dir/src/mesh.cpp.o.provides

CMakeFiles/PA1.dir/src/mesh.cpp.o.provides.build: CMakeFiles/PA1.dir/src/mesh.cpp.o

CMakeFiles/PA1.dir/src/scene_parser.cpp.o: CMakeFiles/PA1.dir/flags.make
CMakeFiles/PA1.dir/src/scene_parser.cpp.o: ../src/scene_parser.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/sam/dev/graphics/PA1.RayCasting/student-code/build/CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/PA1.dir/src/scene_parser.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/PA1.dir/src/scene_parser.cpp.o -c /Users/sam/dev/graphics/PA1.RayCasting/student-code/src/scene_parser.cpp

CMakeFiles/PA1.dir/src/scene_parser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/PA1.dir/src/scene_parser.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /Users/sam/dev/graphics/PA1.RayCasting/student-code/src/scene_parser.cpp > CMakeFiles/PA1.dir/src/scene_parser.cpp.i

CMakeFiles/PA1.dir/src/scene_parser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/PA1.dir/src/scene_parser.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /Users/sam/dev/graphics/PA1.RayCasting/student-code/src/scene_parser.cpp -o CMakeFiles/PA1.dir/src/scene_parser.cpp.s

CMakeFiles/PA1.dir/src/scene_parser.cpp.o.requires:
.PHONY : CMakeFiles/PA1.dir/src/scene_parser.cpp.o.requires

CMakeFiles/PA1.dir/src/scene_parser.cpp.o.provides: CMakeFiles/PA1.dir/src/scene_parser.cpp.o.requires
	$(MAKE) -f CMakeFiles/PA1.dir/build.make CMakeFiles/PA1.dir/src/scene_parser.cpp.o.provides.build
.PHONY : CMakeFiles/PA1.dir/src/scene_parser.cpp.o.provides

CMakeFiles/PA1.dir/src/scene_parser.cpp.o.provides.build: CMakeFiles/PA1.dir/src/scene_parser.cpp.o

# Object files for target PA1
PA1_OBJECTS = \
"CMakeFiles/PA1.dir/src/image.cpp.o" \
"CMakeFiles/PA1.dir/src/main.cpp.o" \
"CMakeFiles/PA1.dir/src/mesh.cpp.o" \
"CMakeFiles/PA1.dir/src/scene_parser.cpp.o"

# External object files for target PA1
PA1_EXTERNAL_OBJECTS =

../bin/PA1: CMakeFiles/PA1.dir/src/image.cpp.o
../bin/PA1: CMakeFiles/PA1.dir/src/main.cpp.o
../bin/PA1: CMakeFiles/PA1.dir/src/mesh.cpp.o
../bin/PA1: CMakeFiles/PA1.dir/src/scene_parser.cpp.o
../bin/PA1: CMakeFiles/PA1.dir/build.make
../bin/PA1: deps/vecmath/libvecmath.a
../bin/PA1: CMakeFiles/PA1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ../bin/PA1"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/PA1.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/PA1.dir/build: ../bin/PA1
.PHONY : CMakeFiles/PA1.dir/build

CMakeFiles/PA1.dir/requires: CMakeFiles/PA1.dir/src/image.cpp.o.requires
CMakeFiles/PA1.dir/requires: CMakeFiles/PA1.dir/src/main.cpp.o.requires
CMakeFiles/PA1.dir/requires: CMakeFiles/PA1.dir/src/mesh.cpp.o.requires
CMakeFiles/PA1.dir/requires: CMakeFiles/PA1.dir/src/scene_parser.cpp.o.requires
.PHONY : CMakeFiles/PA1.dir/requires

CMakeFiles/PA1.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/PA1.dir/cmake_clean.cmake
.PHONY : CMakeFiles/PA1.dir/clean

CMakeFiles/PA1.dir/depend:
	cd /Users/sam/dev/graphics/PA1.RayCasting/student-code/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/sam/dev/graphics/PA1.RayCasting/student-code /Users/sam/dev/graphics/PA1.RayCasting/student-code /Users/sam/dev/graphics/PA1.RayCasting/student-code/build /Users/sam/dev/graphics/PA1.RayCasting/student-code/build /Users/sam/dev/graphics/PA1.RayCasting/student-code/build/CMakeFiles/PA1.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/PA1.dir/depend

