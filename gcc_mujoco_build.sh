#https://mujoco.readthedocs.io/en/latest/programming.html#building-mujoco-from-source
#https://mujoco.readthedocs.io/en/latest/python.html#building-from-source
#https://github.com/deepmind/mujoco/blob/main/.github/workflows/build.yml

# NOTE: In order to expose mjCModel, add MJAPI (requiring #include <mujoco/mjexport.h>) to classes & funcs in src/user header files
# Ref: https://github.com/deepmind/mujoco/issues/388

mkdir -p build_gcc
pushd build_gcc
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/1_MUJOCO/mujoco/release_gcc \
	 -DBUILD_SHARED_LIBS:BOOL=ON
cmake --build . -j8
cmake --install .
popd

# To run binaries (simulate) if using plugins
MJ_RELEASE_PLUGIN_DIR=./release_gcc/bin/mujoco_plugin
rm -rf $MJ_RELEASE_PLUGIN_DIR
mkdir -p $MJ_RELEASE_PLUGIN_DIR
cp ./build_gcc/lib/lib*.so $MJ_RELEASE_PLUGIN_DIR
ls $MJ_RELEASE_PLUGIN_DIR


