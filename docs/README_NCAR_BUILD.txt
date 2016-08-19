Building using the NCAR make system
====================================

1. Set the env var for the install directory:

  LROSE_INSTALL_DIR

2. Set the env var for the core distribution

  LROSE_CORE_DIR

3. Source the environment, depending on the shell you are using:

  cd $LROSE_CORE_DIR

  source build/set_build_env.sh

for sh or bash, or

  source build/set_build_env.csh

for csh or tcsh.

4. Installing the makefiles

The `make` application can use makefiles named either `Makefile` or `makefile`.
The lower-case version takes preference.

The codebase is checked in with upper-case Makefiles throughout the tree.

To get the build you want, you must install the lower-case makefiles relevant
to the distribution you need.

To install the lrose standard distribution makefiles, perform the following:

  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py

  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py --distro lrose

If you want to perform a partial build for a particular distribution,
you can specify the distribution on the command line.

For example, to install the makefile for the radx distribtion, run the following:

  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py --distro radx

4. Build tdrp_gen

  cd $LROSE_CORE_DIR/codebase/libs/tdrp
  make opt
  make install

  cd $LROSE_CORE_DIR/codebase/apps/tdrp/src/tdrp_gen
  make opt
  make install

5. Build the libs:

  cd $LROSE_CORE_DIR/codebase/libs
  make -j 8 install_include
  make -j 8 opt
  make -j 8 install

6. Build the apps:

  cd $LROSE_CORE_DIR/codebase/apps
  make -j 8 opt
  make -j 8 install






