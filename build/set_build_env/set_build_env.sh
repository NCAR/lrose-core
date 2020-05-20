# Set environment variables for building LROSE
# for sh and bash
# Source this file before the build

# Operating system

export HOST_OS=LINUX_LROSE

# directories

export LROSE_CORE_DIR=$HOME/git/lrose-core
export LROSE_INSTALL_DIR=$HOME/lrose

# path

export PATH=$LROSE_INSTALL_DIR/bin:$PATH
export LD_LIBRARY_PATH=$LROSE_INSTALL_DIR/lib:$LD_LIBRARY_PATH
