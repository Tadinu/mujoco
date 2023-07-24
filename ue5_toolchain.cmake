if (NOT DEFINED ENV{UE_ROOT_DIR})
	message (FATAL_ERROR
		"An Unreal Engine root path is needed in order to build 3rd libs for UE."
		"Please provide it through the UE_ROOT_DIR environmental variable.")
endif()

if (NOT DEFINED ENV{UE_CLANG_VER})
	message (FATAL_ERROR
		"An Unreal Engine toolchain version is needed in order to build 3rd libs for UE."
		"Please provide it through the UE_CLANG_VER environmental variable, like v20_clang-13.0.1-centos7 (UE5)")
endif()

set(UE_ROOT_DIR "$ENV{UE_ROOT_DIR}")
set(UE_CLANG_VER "$ENV{UE_CLANG_VER}")

set(UE_TOOLCHAIN_DIR "${UE_ROOT_DIR}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/${UE_CLANG_VER}/x86_64-unknown-linux-gnu")

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_SYSROOT ${UE_TOOLCHAIN_DIR})
#set(CMAKE_FIND_ROOT_PATH "${UE_TOOLCHAIN_DIR}/usr")
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_COMPILER ${UE_TOOLCHAIN_DIR}/bin/clang CACHE PATH "clang" FORCE)
set(CMAKE_CXX_COMPILER ${UE_TOOLCHAIN_DIR}/bin/clang++ CACHE PATH "clang++" FORCE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17 -stdlib=libc++") #-march=native -mzvector
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")

# ${UE_TOOLCHAIN_DIR}/include/c++/4.8.5
# System [libstdc++] (should not be used due to abi incompatibility): 
# /usr/include:/usr/include/x86_64-linux-gnu:/usr/include/c++/9:/usr/include/c++/9/experimental:/usr/include/x86_64-linux-gnu/c++/9
# System [libc++] (might be probably used if UE toolchain headers does not suffice): /usr/include/clang/11/include
include_directories(SYSTEM "${UE_TOOLCHAIN_DIR}/usr/include/:${UE_TOOLCHAIN_DIR}/usr/include/linux/:${UE_ROOT_DIR}/Engine/Source/ThirdParty/Unix/LibCxx/include/c++/v1/:${UE_TOOLCHAIN_DIR}/include/c++/4.8.5")

link_directories("${UE_TOOLCHAIN_DIR}/lib64" "${UE_TOOLCHAIN_DIR}/usr/lib64" "${UE_ROOT_DIR}/Engine/Source/ThirdParty/Unix/LibCxx/lib/Unix/x86_64-unknown-linux-gnu")
#get_property(lib_path DIRECTORY PROPERTY LINK_DIRECTORIES)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure paths for find_package() used by all 3rd party libs
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_LIST_DIR}/tinyxml2/release
										   ${CMAKE_CURRENT_LIST_DIR}/eigen/release
										   ${CMAKE_CURRENT_LIST_DIR}/ign-cmake/release
										   ${CMAKE_CURRENT_LIST_DIR}/ign-tools/release
										   ${CMAKE_CURRENT_LIST_DIR}/ign-utils/release
										   ${CMAKE_CURRENT_LIST_DIR}/ign-math/release)
#message("ue_toolchain_cmake CMAKE_PREFIX_PATH:" ${CMAKE_PREFIX_PATH})


#set(CMAKE_EXE_LINKER_FLAGS_INIT
#    "-L${CMAKE_SYSROOT}/opt/ros/noetic/lib -Wl,-rpath,${CMAKE_SYSROOT}/opt/ros/noetic/lib"
#)
#set(CMAKE_SHARED_LINKER_FLAGS_INIT
#    "-L${CMAKE_SYSROOT}/opt/ros/noetic/lib -Wl,-rpath,${CMAKE_SYSROOT}/opt/ros/noetic/lib"
#)
