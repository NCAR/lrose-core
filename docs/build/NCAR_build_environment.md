# Building using the NCAR development environment

Setting up the NCAR development environmant allows a user to
develop LROSE-specific code in an efficient environment.

It uses simple Makefiles, rather than the complex makefiles generated
by autoconf and automake.

## Setting up your environment

The software development system at NCAR/RAL (formerly RAP) and NCAR/EOL makes use of a recursive makefile approach, using environment variables to identify the various directories used during the build.

Therefore, before performing the build, you need to set up the correct environment, as follows:

### Set the environment variable for the directory into which you wish to install the build:

```
  $LROSE_INSTALL_DIR
```

This will normally be:

```
  ~/lrose
```

The build will be installed in:

```
  $LROSE_INSTALL_DIR/bin
  $LROSE_INSTALL_DIR/lib
  $LROSE_INSTALL_DIR/include
```

### Set the environment variable to point to the git lrose-core directory:

```
  $LROSE_CORE_DIR
```

This should point to the top of the distribution, i.e. lrose-core.

This will normally be:

```
  ~/git/lrose-core
```

### Source the environment, depending on the shell you are using:

For sh or bash:
```
  cd $LROSE_CORE_DIR
  source build/set_build_env.sh
```  

For csh or tcsh:
```
  cd $LROSE_CORE_DIR
  source build/set_build_env.csh
```

You can insert these files directly into your `.cshrc` or `.bashrc` file, so that the environment is always set.

This will set the following important environment variables:

```
 $RAP_MAKE_INC_DIR: include files used by the makefiles
 $RAP_MAKE_BIN_DIR: scripts for the make
 $RAP_INC_DIR: the include install directory
 $RAP_LIB_DIR: the library install directory
 $RAP_BIN_DIR: the binary install directory
```

Several other variables are set as well.

## Check out, build and install **netcdf** support

See [NCAR_netcdf_build.md](./NCAR_netcdf_build.md)

Install in $LROSE_INSTALL_DIR

## Installing the makefiles

The `make` application can use makefiles named either `Makefile` or `makefile`.
The lower-case version takes preference.

The codebase is checked in with upper-case Makefiles throughout the tree.

To get the build you want, you must install the lower-case makefiles relevant to the package you need.

To install the **lrose** standard package makefiles, perform the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_package_makefiles.py
```
This is equivalent to the following

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_package_makefiles.py --package lrose
```

If you want to perform a specific package package, you can specify that on the command line.

For the **radx** distribtion, run the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_package_makefiles.py --package radx
```

For the **titan** distribtion, run the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_package_makefiles.py --package titan
```

## Performing the build

### (a) Build and install the TDRP parameter handling utility

```
  cd $LROSE_CORE_DIR/codebase/libs/tdrp/src
  make opt install
  cd $LROSE_CORE_DIR/codebase/apps/tdrp/src
  make opt install
```

### (b) Build and install the libraries

```
  cd $LROSE_CORE_DIR/codebase/libs/
  make install_include
  make -j 8 opt
  make -j 8 install
```

### (c) Build and instal the applications

```
  cd $LROSE_CORE_DIR/codebase/apps
  make -j 8 opt
  make -j 8 install
```

## Building individual applications

Once you have set up the environment specified above, you are free
to edit and build individual applications.

For example, if you want to work on RadxConvert, you would go
to the relevant directory and perform the build locally there.

```
  cd $LROSE_CORE_DIR/codebase/apps/Radx/src/RadxConvert
  make clean
  make opt
  make install
```


