Building using the NCAR make system
====================================

1. Set the env var for the install directory:

  LROSE_INSTALL_DIR

2. Set the env var for the core distribution

  LROSE_CORE_DIR

3. Source the environment, depending on the shell you are using:

  cd $LROSE_CORE_DIR/codebase

  source build/set_build_env.sh

for sh or bash, or

  source build/set_build_env.csh

for csh or tcsh.

4. Build the libs:

  cd libs
  make -j 8 install_include
  make -j 8 opt
  make -j 8 install

5. Build the apps:

  cd ../apps
  make -j 8 opt
  make -j 8 install






