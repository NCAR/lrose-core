# Building using the NCAR development environment - LINUX

Setting up the NCAR development environmant allows a user to
develop LROSE-specific code in an efficient environment.

It uses Makefiles that are simple to use, rather than the complex makefiles generated
by the GNU tools autoconf and automake.

1. [prepare](#prepare)
2. [download](#download)
3. [setenv](#setenv)
4. [build](#build)

<a name="prepare"/>

## 1. Prepare the OS

Most good, up-to date LINUX distributions should work.

Recommended distributions are:

  * Debian
  * Ubuntu (based on Debian)
  * RedHat
  * Centos (based on RedHat)
  * Fedora (based on RedHat)

First, you will need to install the required packages.

See: [LROSE package dependencies](./lrose_package_dependencies.md)

<a name="download"/>

## 2. Download from GitHub

Create a working directory for cloning:

```
  mkdir -p ~/git
  cd ~/git
```

### Clone the current LROSE version from GitHub

```
  cd ~/git
  git clone https://github.com/ncar/lrose-core 
```

The distribution will be in the lrose-core subdirectory:

```
  cd ~/git/lrose-core
```

### Clone the NetCDF support from GitHub - if necessary

This step is generally no longer required, provided the netcdf-devel and hdf5-devel package dependencies are installed.

See: [LROSE package dependencies](./lrose_package_dependencies.md)

```
  cd ~/git
  git clone https://github.com/ncar/lrose-netcdf
```

<a name="setenv"/>

## 3. Setting up your environment

The LROSE manual build uses a recursive makefile approach, using environment variables to identify the various directories used during the build.

Therefore, before performing the build, you need to set up the correct environment.

### Set environment variables for the build:

For sh or bash:
```
export HOST_OS=LINUX_LROSE
export LROSE_INSTALL_DIR=$HOME/lrose
export LROSE_CORE_DIR=$HOME/git/lrose-core
```

For csh or tcsh:
```
setenv HOST_OS OSX_LROSE
setenv LROSE_INSTALL_DIR $HOME/lrose
setenv LROSE_CORE_DIR $HOME/git/lrose-core
```

Preferably, you should permanently set these directly in your `.cshrc` or `.bashrc` file.
Then the environment will always be correctly set.

The software is installed in:
```
  $LROSE_INSTALL_DIR/bin
  $LROSE_INSTALL_DIR/lib
  $LROSE_INSTALL_DIR/include
```
 
<a name="build"/>

## 4. Build

### First build the NetCDF support - if necessary.

This step is generally no longer required, provided the netcdf-devel and hdf5-devel package dependencies are installed.

See: [LROSE package dependencies](./lrose_package_dependencies.md)

To do the build see [NCAR_netcdf_build.linux.md](./NCAR_netcdf_build.linux.md)

Install NetCDF in $LROSE_INSTALL_DIR, which will normally be `~/lrose`.

### Install the makefiles

The `make` application can use makefiles named either `Makefile` or `makefile`.
The lower-case version takes preference.

The codebase is checked in with upper-case Makefiles throughout the tree.

To get the build you want, you must install the lower-case makefiles relevant to the package you need.

To install the **lrose-core** standard package makefiles, perform the following:

```
  cd $LROSE_CORE_DIR
  ./build/scripts/installPackageMakefiles.py
```
This is equivalent to the following

```
  cd $LROSE_CORE_DIR
  ./build/scripts/installPackageMakefiles.py --package lrose-core
```

If you want to build a specific version, you can specify that on the command line.

For the **titan** distribtion, run the following:

```
  cd $LROSE_CORE_DIR
  ./build/scripts/installPackageMakefiles.py --package titan
```

For the **samurai** distribtion, run the following:

```
  cd $LROSE_CORE_DIR
  ./build/scripts/installPackageMakefiles.py --package samurai
```

### Perform the build

#### (a) Build and install the TDRP parameter handling utility

```
  cd $LROSE_CORE_DIR/codebase/libs/tdrp/src
  make opt install
  cd $LROSE_CORE_DIR/codebase/apps/tdrp/src/tdrp_gen
  make opt install
```

#### (b) Build and install the libraries

```
  cd $LROSE_CORE_DIR/codebase/libs/
  make -j 8 install_include
  make -j 8 opt
  make -j 8 install
```

#### (c) Build and instal the applications

```
  cd $LROSE_CORE_DIR/codebase/apps
  make -j 8 opt
  make -j 8 install
```

### Building individual applications

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


