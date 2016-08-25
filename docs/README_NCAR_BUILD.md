## Building using the NCAR development system

### Setting up the environment

The software development system at NCAR/RAL (formerly RAP) and NCAR/EOL makes use of a recursive makefile approach, using environment variables to identify the various directories used during the build.

Therefore, before performing the build, you need to set up the correct environment, as follows:

##### Set the environment variable for the directory into which you wish to install the build:

```
  $LROSE_INSTALL_DIR
```

The build will be installed in:

```
  $LROSE_INSTALL_DIR/bin
  $LROSE_INSTALL_DIR/lib
  $LROSE_INSTALL_DIR/include
```

##### Set the environment variable to point to the git lrose-core directory:

```
  $LROSE_CORE_DIR
```

This should point to the top of the distribution, i.e. lrose-core.

##### Source the environment, depending on the shell you are using:

```
  cd $LROSE_CORE_DIR
  source build/set_build_env.sh
```  

for sh or bash, or

```
  cd $LROSE_CORE_DIR
  source build/set_build_env.csh
```

for csh or tcsh.

This will set the following important environnent variables:

```
 $RAP_MAKE_INC_DIR: include files used by the makefiles
 $RAP_MAKE_BIN_DIR: scripts for the make
 $RAP_INC_DIR: the include install directory
 $RAP_LIB_DIR: the library install directory
 $RAP_BIN_DIR: the binary install directory
```

### Installing HDF5 and NetCDF

To install the correct HDF5 and NetCDF support, you need to download lrose-netcdf:

[https://github.com/NCAR/lrose-netcdf]

Follow the instructions for performing the build.

Install the build in:

```
  $LROSE_INSTALL_DIR
```

### Installing the makefiles

The `make` application can use makefiles named either `Makefile` or `makefile`.
The lower-case version takes preference.

The codebase is checked in with upper-case Makefiles throughout the tree.

To get the build you want, you must install the lower-case makefiles relevant to the distribution you need.

To install the **lrose** standard distribution makefiles, perform the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py
```
This is equivalent to the following

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py --distro lrose
```

If you want to perform a partial build for a sub distribution, you can specify that on the command line.

For the **radx** distribtion, run the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py --distro radx
```

For the **cidd** distribtion, run the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py --distro cidd
```

For the **hcr** distribtion, run the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py --distro hcr
```

### Performing the build

#### Building and installing the TDRP parameter handling utility

```
  cd $LROSE_CORE_DIR/codebase/libs/tdrp/src
  make opt install
  cd $LROSE_CORE_DIR/codebase/apps/tdrp/src
  make opt install
```

#### Building and installing the libraries

```
  cd $LROSE_CORE_DIR/codebase/libs/
  make install_include
  make -j 8 opt
  make install
```

#### Building and installing the applications

```
  cd $LROSE_CORE_DIR/codebase/apps
  make -j 8 opt
  make install
```

