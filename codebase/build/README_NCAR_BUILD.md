# Building using the NCAR development system

### Setting up the environment

The software development system at NCAR/RAL (formerly RAP) and NCAR/EOL makes use of a recursive makefile approach, using environment variables to identify the various directories used during the build.

Therefore, before performing the build, you need to set up the correct environment, as follows:

1. Set the environment variable for the directory into which you wish to install the build:

  LROSE_INSTALL_DIR
  
The build will be installed in:

  $LROSE_INSTALL_DIR/bin
  $LROSE_INSTALL_DIR/lib
  $LROSE_INSTALL_DIR/include

2. Set the environment variable to point to the git lrose-core directory:

  LROSE_CORE_DIR
  
This should point to the top of the distribution, i.e. lrose-core.

3. Source the environment, depending on the shell you are using:

  cd $LROSE_CORE_DIR/codebase

  source build/set_build_env.sh

for sh or bash, or

  source build/set_build_env.csh

for csh or tcsh.

This will set the following important environnent variables:

 $RAP_MAKE_INC_DIR: include files used by the makefiles
 $RAP_MAKE_BIN_DIR: scripts for the make
 $RAP_INC_DIR: the include install directory
 $RAP_LIB_DIR: the library install directory
 $RAP_BIN_DIR: the binary install directory



4. Build the libs:

  cd libs
  make -j 8 install_include
  make -j 8 opt
  make -j 8 install

5. Build the apps:

  cd ../apps
  make -j 8 opt
  make -j 8 install






