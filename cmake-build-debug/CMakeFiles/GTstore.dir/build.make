# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_COMMAND = /home/chenzy/clion/bin/cmake/bin/cmake

# The command to remove a file.
RM = /home/chenzy/clion/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/chenzy/Desktop/Code/OS/GTstore

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/chenzy/Desktop/Code/OS/GTstore/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/GTstore.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/GTstore.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/GTstore.dir/flags.make

CMakeFiles/GTstore.dir/test_gt_manager.cpp.o: CMakeFiles/GTstore.dir/flags.make
CMakeFiles/GTstore.dir/test_gt_manager.cpp.o: ../test_gt_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/chenzy/Desktop/Code/OS/GTstore/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/GTstore.dir/test_gt_manager.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/GTstore.dir/test_gt_manager.cpp.o -c /home/chenzy/Desktop/Code/OS/GTstore/test_gt_manager.cpp

CMakeFiles/GTstore.dir/test_gt_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/GTstore.dir/test_gt_manager.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/chenzy/Desktop/Code/OS/GTstore/test_gt_manager.cpp > CMakeFiles/GTstore.dir/test_gt_manager.cpp.i

CMakeFiles/GTstore.dir/test_gt_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/GTstore.dir/test_gt_manager.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/chenzy/Desktop/Code/OS/GTstore/test_gt_manager.cpp -o CMakeFiles/GTstore.dir/test_gt_manager.cpp.s

CMakeFiles/GTstore.dir/test_gt_manager.cpp.o.requires:

.PHONY : CMakeFiles/GTstore.dir/test_gt_manager.cpp.o.requires

CMakeFiles/GTstore.dir/test_gt_manager.cpp.o.provides: CMakeFiles/GTstore.dir/test_gt_manager.cpp.o.requires
	$(MAKE) -f CMakeFiles/GTstore.dir/build.make CMakeFiles/GTstore.dir/test_gt_manager.cpp.o.provides.build
.PHONY : CMakeFiles/GTstore.dir/test_gt_manager.cpp.o.provides

CMakeFiles/GTstore.dir/test_gt_manager.cpp.o.provides.build: CMakeFiles/GTstore.dir/test_gt_manager.cpp.o


CMakeFiles/GTstore.dir/gt_storage_node.cpp.o: CMakeFiles/GTstore.dir/flags.make
CMakeFiles/GTstore.dir/gt_storage_node.cpp.o: ../gt_storage_node.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/chenzy/Desktop/Code/OS/GTstore/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/GTstore.dir/gt_storage_node.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/GTstore.dir/gt_storage_node.cpp.o -c /home/chenzy/Desktop/Code/OS/GTstore/gt_storage_node.cpp

CMakeFiles/GTstore.dir/gt_storage_node.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/GTstore.dir/gt_storage_node.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/chenzy/Desktop/Code/OS/GTstore/gt_storage_node.cpp > CMakeFiles/GTstore.dir/gt_storage_node.cpp.i

CMakeFiles/GTstore.dir/gt_storage_node.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/GTstore.dir/gt_storage_node.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/chenzy/Desktop/Code/OS/GTstore/gt_storage_node.cpp -o CMakeFiles/GTstore.dir/gt_storage_node.cpp.s

CMakeFiles/GTstore.dir/gt_storage_node.cpp.o.requires:

.PHONY : CMakeFiles/GTstore.dir/gt_storage_node.cpp.o.requires

CMakeFiles/GTstore.dir/gt_storage_node.cpp.o.provides: CMakeFiles/GTstore.dir/gt_storage_node.cpp.o.requires
	$(MAKE) -f CMakeFiles/GTstore.dir/build.make CMakeFiles/GTstore.dir/gt_storage_node.cpp.o.provides.build
.PHONY : CMakeFiles/GTstore.dir/gt_storage_node.cpp.o.provides

CMakeFiles/GTstore.dir/gt_storage_node.cpp.o.provides.build: CMakeFiles/GTstore.dir/gt_storage_node.cpp.o


# Object files for target GTstore
GTstore_OBJECTS = \
"CMakeFiles/GTstore.dir/test_gt_manager.cpp.o" \
"CMakeFiles/GTstore.dir/gt_storage_node.cpp.o"

# External object files for target GTstore
GTstore_EXTERNAL_OBJECTS =

GTstore: CMakeFiles/GTstore.dir/test_gt_manager.cpp.o
GTstore: CMakeFiles/GTstore.dir/gt_storage_node.cpp.o
GTstore: CMakeFiles/GTstore.dir/build.make
GTstore: CMakeFiles/GTstore.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/chenzy/Desktop/Code/OS/GTstore/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable GTstore"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/GTstore.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/GTstore.dir/build: GTstore

.PHONY : CMakeFiles/GTstore.dir/build

CMakeFiles/GTstore.dir/requires: CMakeFiles/GTstore.dir/test_gt_manager.cpp.o.requires
CMakeFiles/GTstore.dir/requires: CMakeFiles/GTstore.dir/gt_storage_node.cpp.o.requires

.PHONY : CMakeFiles/GTstore.dir/requires

CMakeFiles/GTstore.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/GTstore.dir/cmake_clean.cmake
.PHONY : CMakeFiles/GTstore.dir/clean

CMakeFiles/GTstore.dir/depend:
	cd /home/chenzy/Desktop/Code/OS/GTstore/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/chenzy/Desktop/Code/OS/GTstore /home/chenzy/Desktop/Code/OS/GTstore /home/chenzy/Desktop/Code/OS/GTstore/cmake-build-debug /home/chenzy/Desktop/Code/OS/GTstore/cmake-build-debug /home/chenzy/Desktop/Code/OS/GTstore/cmake-build-debug/CMakeFiles/GTstore.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/GTstore.dir/depend

