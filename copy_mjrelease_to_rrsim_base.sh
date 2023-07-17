DEST_RELEASE_DIR=${1:-"/home/tad/0_UE/UE5_FLAPTTER/Plugins/RRSimBase/ThirdParty/mujoco/release"}
DEST_INCL_DIR="$DEST_RELEASE_DIR/include/"
rm -rf $DEST_INCL_DIR
mkdir -p $DEST_INCL_DIR
cp -a include/mujoco $DEST_INCL_DIR
cp build/_deps/lodepng-src/lodepng.h $DEST_INCL_DIR

# Plugins
DEST_MUJOCO_PLUGIN_INCL_DIR=$DEST_INCL_DIR/mujoco/plugin
rm -f $DEST_MUJOCO_PLUGIN_INCL_DIR
mkdir -p $DEST_MUJOCO_PLUGIN_INCL_DIR/elasticity
cp -f plugin/elasticity/*.h $DEST_MUJOCO_PLUGIN_INCL_DIR/elasticity

DEST_LIB_DIR="$DEST_RELEASE_DIR/lib/"
rm -rf $DEST_LIB_DIR
mkdir -p $DEST_LIB_DIR
cp -f release/lib/libmujoco.so $DEST_LIB_DIR 
cp -f release/lib/libmujoco.so.3.1.3 $DEST_LIB_DIR

# Plugins
cp -f build/lib/*.so $DEST_LIB_DIR
ls -al $DEST_LIB_DIR
