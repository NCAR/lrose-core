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

On Redhat-based hosts, run the following (as root or sudo):

```
sudo yum install -y epel-release

sudo yum install -y \
tcsh perl perl-Env ftp git svn cvs tkcvs emacs tkcvs m4 \
gcc gcc-c++ gcc-gfortran glibc-devel libX11-devel libXext-devel \
libpng-devel libtiff-devel zlib-devel expat-devel libcurl-devel \
flex-devel fftw3-devel bzip2-devel jasper-devel qt5-qtbase-devel xrdb \
Xvfb xorg-x11-fonts-misc xorg-x11-fonts-75dpi xorg-x11-fonts-100dpi \
gnuplot ImageMagick-devel ImageMagick-c++-devel

cd /usr/bin; sudo ln -s qmake-qt5 qmake

```

On Debian-based hosts run the following (as root or sudo):

```
sudo apt-get update 

sudo apt-get install -y  \
    libbz2-dev libx11-dev libpng-dev libfftw3-dev \
    libjasper-dev qtbase5-dev git \
    gcc g++ gfortran libfl-dev \
    automake make libtool pkg-config libexpat1-dev python

cd /usr/bin; sudo ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake

```

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

### Clone the NetCDF support rom GitHub

```
  cd ~/git
  git clone https://github.com/ncar/lrose-netcdf
```

<a name="setenv"/>

## 3. Setting up your environment

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

Preferably, you should permanently copy the contents of these these files
directly into your `.cshrc` or `.bashrc` file.
That way the environment will always be correcty set.

This will set the following important environment variables:

```
 $HOST_OS: the flavor of OS for your system.
 $RAP_MAKE_INC_DIR: include files used by the makefiles
 $RAP_MAKE_BIN_DIR: scripts for the make
 $RAP_INC_DIR: the include install directory
 $RAP_LIB_DIR: the library install directory
 $RAP_BIN_DIR: the binary install directory
```

Several other variables are set as well.

<a name="build"/>

## 4. Build

### First build the NetCDF support.

See [NCAR_netcdf_build.linux.md](./NCAR_netcdf_build.linux.md)

Install NetCDF in $LROSE_INSTALL_DIR, which will normally be `~/lrose`.

### Install the makefiles

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

### Perform the build

#### (a) Build and install the TDRP parameter handling utility

```
  cd $LROSE_CORE_DIR/codebase/libs/tdrp/src
  make opt install
  cd $LROSE_CORE_DIR/codebase/apps/tdrp/src
  make opt install
```

#### (b) Build and install the libraries

```
  cd $LROSE_CORE_DIR/codebase/libs/
  make install_include
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


