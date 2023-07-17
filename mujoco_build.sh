#https://mujoco.readthedocs.io/en/latest/programming.html#building-mujoco-from-source
#https://mujoco.readthedocs.io/en/latest/python.html#building-from-source
#https://github.com/deepmind/mujoco/blob/main/.github/workflows/build.yml

# NOTE: In order to expose mjCModel, add MJAPI (requiring #include <mujoco/mjexport.h>) to classes & funcs in src/user header files
# Ref: https://github.com/deepmind/mujoco/issues/388

# Ubuntu 20.04: clang-10
# Ubuntu 22.04: clang-11/12/13
#sudo apt update
#sudo apt install libc++-dev
#sudo apt install libc++abi-dev
mkdir -p build
pushd build
cmake .. -DCMAKE_C_COMPILER:STRING=clang-15 -DCMAKE_CXX_COMPILER:STRING=clang++-15 -DMUJOCO_HARDEN:BOOL=ON \
		 -DCMAKE_INSTALL_PREFIX=$HOME/1_MUJOCO/mujoco/release \
		 -DCMAKE_CXX_FLAGS:STRING="-stdlib=libc++ -D_GLIBCXX_USE_CXX11_ABI=1" \
		 -DCMAKE_EXE_LINKER_FLAGS:STRING="-Wl,--no-as-needed -stdlib=libc++" \
		 -DBUILD_SHARED_LIBS:BOOL=ON
cmake --build . -j8
cmake --install .
popd

# To run binaries (simulate) if using plugins
MJ_RELEASE_PLUGIN_DIR=./release/bin/mujoco_plugin
rm -rf $MJ_RELEASE_PLUGIN_DIR
mkdir -p $MJ_RELEASE_PLUGIN_DIR
cp ./build/lib/lib*.so $MJ_RELEASE_PLUGIN_DIR
ls $MJ_RELEASE_PLUGIN_DIR

# Copy mujoco release (libs + headers) to UE's RRSimBase
source ./copy_mjrelease_to_rrsim_base.sh


