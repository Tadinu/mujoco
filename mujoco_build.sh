#https://mujoco.readthedocs.io/en/latest/programming.html#building-mujoco-from-source
#https://mujoco.readthedocs.io/en/latest/python.html#building-from-source
#https://github.com/deepmind/mujoco/blob/main/.github/workflows/build.yml

# Ubuntu 20.04: clang-10
# Ubuntu 22.04: clang-11/12/13
sudo apt install libc++-dev
sudo apt install libc++abi-dev
mkdir -p build
pushd build
cmake .. -DCMAKE_C_COMPILER:STRING=clang-10 -DCMAKE_CXX_COMPILER:STRING=clang++-10 -DMUJOCO_HARDEN:BOOL=ON \
		 -DCMAKE_INSTALL_PREFIX=../release -D_GLIBCXX_USE_CXX11_ABI=1 \
		 -DCMAKE_CXX_FLAGS:STRING="-pthread -std=c++17 -stdlib=libc++" \
		 -DCMAKE_EXE_LINKER_FLAGS:STRING="-Wl,--no-as-needed -stdlib=libc++"
cmake --build .
cmake --install .
popd


