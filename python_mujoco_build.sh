MJ_VERSION=$1
export WORKON_HOME=~/.virtualenvs
VIRTUALENVWRAPPER_PYTHON='/usr/bin/python3'
workon mujoco
cd python
export MUJOCO_PATH=/home/tad/1_MUJOCO/mujoco/release
export MUJOCO_PLUGIN_PATH=/home/tad/1_MUJOCO/mujoco/release/bin/mujoco_plugin
bash make_sdist.sh
cd dist
pip install mujoco-${MJ_VERSION}.tar.gz
