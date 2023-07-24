#https://mujoco.readthedocs.io/en/latest/programming.html#building-mujoco-from-source
#https://mujoco.readthedocs.io/en/latest/python.html#building-from-source
#https://github.com/deepmind/mujoco/blob/main/.github/workflows/build.yml

# Ubuntu 20.04: clang-10
# Ubuntu 22.04: clang-11/12/13
sudo apt install libc++-dev
sudo apt install libc++abi-dev

UE_TOOLCHAIN_DIR="${UE5_DIR}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v20_clang-13.0.1-centos7/x86_64-unknown-linux-gnu"
CLANG="${UE_TOOLCHAIN_DIR}/bin/clang" # or clang-10
CLANG_PP="${UE_TOOLCHAIN_DIR}/bin/clang++" # or clang-10
mkdir -p build
pushd build
cmake .. -DCMAKE_C_COMPILER:STRING="${CLANG}" -DCMAKE_CXX_COMPILER:STRING="${CLANG_PP}" -DMUJOCO_HARDEN:BOOL=ON \
		 -DCMAKE_INSTALL_PREFIX=../release \ #-D_GLIBCXX_USE_CXX11_ABI=1
		 -DCMAKE_CXX_FLAGS:STRING="-pthread -stdlib=libc++" \
		 -DCMAKE_EXE_LINKER_FLAGS:STRING="-Wl,--no-as-needed -stdlib=libc++" \
		 -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	     -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		 -DCMAKE_SYSTEM_INCLUDE_PATH="${UE_TOOLCHAIN_DIR}/usr/include/ \
                                      ${UE_TOOLCHAIN_DIR}/usr/include/linux/ \
									  ${UE_TOOLCHAIN_DIR}/include/c++/4.8.5/" \
									  ${UE_TOOLCHAIN_DIR}/include/c++/4.8.5/x86_64-unknown-linux-gnu/ \
									  ${UE5_DIR}/Engine/Source/ThirdParty/Unix/LibCxx/include/c++/v1/ \
		 -DCMAKE_SYSTEM_LIBRARY_PATH="${UE_TOOLCHAIN_DIR}/lib64" "${UE_TOOLCHAIN_DIR}/usr/lib64" "${UE5_DIR}/Engine/Source/ThirdParty/Unix/LibCxx/lib/Unix/x86_64-unknown-linux-gnu"
cmake --build .
cmake --install .
popd


