#!/usr/bin/env bash

if [[ "$#" -lt 1 ]]; then
	printf "The deployment script needs to know the path of the Unreal Engine project to deploy to.\n"
	exit 1
fi

# Set exit-on-error mode
set -e

# Set exit trap
# https://medium.com/@dirk.avery/the-bash-trap-trap-ce6083f36700
trap 'trap_catch $? ${LINENO}' EXIT
trap_catch() {
	if [ "$1" != "0" ]; then
		printf "Deployment error %u at line %u!\n" "$1" "$2"
		printf "Performing cleanup of intermediate results...\n"
		if [[ -d "${BUILD_DIR}" ]]; then
			rm -rf "${BUILD_DIR}"
		fi	
		exit $1
	fi
}

# UE4 -> cmake 3.10 only
# Ubuntu 20.04 -> clang-10
UE_ROOT="$1"
UE_CLANG_VER=${2:-"v20_clang-13.0.1-centos7"} # "v19_clang-11.0.1-centos7" or "v20_clang-13.0.1-centos7"
UE_TOOLCHAIN_DIR="${UE_ROOT}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64"
UE_CLANG_PATH="${UE_TOOLCHAIN_DIR}/${UE_CLANG_VER}"
echo "UE_CLANG_PATH: ${UE_CLANG_PATH}"

UE_TOOLCHAIN_LIB64_PATH="$(realpath "${UE_CLANG_PATH}/x86_64-unknown-linux-gnu/lib64")"
UE_TOOLCHAIN_USR_LIB_PATH="${UE_TOOLCHAIN_LIB64_PATH}/../usr/lib64"

#echo "Adding essential libs from ${UE_TOOLCHAIN_USR_LIB_PATH} to ${UE_TOOLCHAIN_LIB64_PATH}"
#ln -s ${UE_TOOLCHAIN_USR_LIB_PATH}/libc.so ${UE_TOOLCHAIN_LIB64_PATH}/libc.so
#ln -s ${UE_TOOLCHAIN_USR_LIB_PATH}/libm.so ${UE_TOOLCHAIN_LIB64_PATH}/libm.so
#ln -s ${UE_TOOLCHAIN_USR_LIB_PATH}/libpthread.so ${UE_TOOLCHAIN_LIB64_PATH}/libpthread.so
#ln -s ${UE_TOOLCHAIN_USR_LIB_PATH}/libc_nonshared.a ${UE_TOOLCHAIN_LIB64_PATH}/libc_nonshared.a
#find ${UE_TOOLCHAIN_USR_LIB_PATH} -name "crt*" -exec cp -t "${UE_TOOLCHAIN_LIB64_PATH}" '{}' +

BUILD_TYPE=${3:-Release}

# Always create a new fresh symbolic link, which could have been outdated
create_link() {
	if [[ -L "$2" ]]; then
		rm "$2"
	fi

	ln -s "$1" "$2"
}

# Base dir as the script's parent dir
BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Go to the base dir
pushd "$BASE_DIR" >/dev/null
#git submodule update mujoco --init --recursive

MUJOCO_DIR="${BASE_DIR}/mujoco"

# BUILD_DIR will be [CMAKE_CURRENT_BINARY_DIR]
BUILD_DIR="${MUJOCO_DIR}/build"
INC_DIR="${MUJOCO_DIR}"
RELEASE_LIB_DIR="${MUJOCO_DIR}/release"

echo "MUJOCO dir: ${MUJOCO_DIR}"

# Create build & release folders
create_folder() {
	if [[ ! -d "$1" ]]; then
		mkdir -p "$1"
	fi
}
create_folder "${BUILD_DIR}"
create_folder "${RELEASE_LIB_DIR}"

echo "Generating Makefiles and building library"
# Build MUJOCO (requiring CMake >= 3.16)
# (UE4 Toolchain still works with cmake3.10 only)
# Install CMake 3.16 at ~/CMAKE/3.16.4: cd ~/CMAKE && cmake-3.16.4-linux-x86_64.sh --prefix=./3.16.4
# https://cmake.org/cmake/help/latest/variable/CMAKE_TOOLCHAIN_FILE.html
# https://stackoverflow.com/questions/5098360/cmake-specifying-build-toolchain
cd "${BUILD_DIR}"
UE_ROOT_DIR="${UE_ROOT}" UE_CLANG_VER="${UE_CLANG_VER}" /usr/bin/cmake "${MUJOCO_DIR}" -DCMAKE_TOOLCHAIN_FILE="${BASE_DIR}/ue_toolchain.cmake" -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
			-DCMAKE_INSTALL_PREFIX=${RELEASE_LIB_DIR}
# -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DBUILD_TESTING=OFF -DBUILD_TESTS=OFF 

VERBOSE=1 make
make install

# Return to the original dir
popd >/dev/null
echo "MUJOCO release: ${RELEASE_LIB_DIR}"
echo "MUJOCO setup complete!"

exit $?
