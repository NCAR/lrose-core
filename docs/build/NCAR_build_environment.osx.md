# Building using the NCAR development environment - OSX

Setting up the NCAR development environmant allows a user to
develop LROSE-specific code in an efficient environment.

It uses Makefiles that are simple to use, rather than the complex makefiles generated
by the GNU tools autoconf and automake.

1. [prepare](#prepare)
2. [download](#download)
3. [set environment variables](#setenv)
4. [build](#build)

<a name="prepare"/>

## 1. Prepare the OS

Install either the XCode development environment or a stand-alone version of the
XCode command line tools.  If you intend to do lots of Apple development and
want to use an IDE, then install XCode.

### Installing complete XCode

To install the full XCode package, get an Apple ID and register for the Apple App Store.

You will need to provide a credit card, so Apple can charge you if you actually buy anything.  
However, XCode is free.

From the App Store, install XCode.
Start XCode, open the preferences window, select the 'Downloads' tab, and 
install "Command Line Tools"

You may also need to run:

```
  xcode-select --install
```

### Installing stand-alone XCode command line tools

Alternatively, you can install the stand-alone XCode command line tools by downloading
"Command Line Tools" from:

```
  http://developer.apple.com/downloads
```

You will need to register for a free Apple id, no credit card is required.

### Install homebrew

The default location for homebrew is /usr/local. So you need write permission
to /usr/local to perform the install.

Run the following ruby script:

```
  /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  /usr/local/bin/brew update
```

### Install required packages, using brew

```
  brew install pkg-config
  brew install szip
  brew install hdf5 --enable-cxx
  brew install netcdf
  brew install udunits
  brew install fftw
  brew install flex
  brew install jasper
  brew install jpeg
  brew install libpng
  brew install qt
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

<a name="setenv"/>

## 3. Setting up your environment

The LROSE manual build uses a recursive makefile approach, using environment variables to identify the various directories used during the build.

Therefore, before performing the build, you need to set up the correct environment.

### Set environment variables for the build:

For sh or bash:
```
export HOST_OS=OSX_LROSE
export LROSE_INSTALL_DIR=/Users/myname/lrose
export LROSE_CORE_DIR=/Users/myname/git/lrose-core
```

For csh or tcsh:
```
setenv HOST_OS OSX_LROSE
setenv LROSE_INSTALL_DIR /Users/myname/lrose
setenv LROSE_CORE_DIR /Users/myname/git/lrose-core
```

Preferably, you should permanently set these directly in your `.cshrc` or `.bashrc` file.
Then the environment will always be correctly set.

The software is installed in:
```
  $LROSE_INSTALL_DIR/bin
  $LROSE_INSTALL_DIR/lib
  $LROSE_INSTALL_DIR/include
```

Note: if there are errors related to Qt, try setting this variable
```
 export PKG_CONFIG_PATH="/usr/local/opt/qt/lib/pkgconfig"
 ```
 
<a name="build"/>

## 4. Build

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

If you want to perform a specific package package, you can specify that on the command line.

For example, for the **samurai** distribtion, run the following:

```
  cd $LROSE_CORE_DIR
  ./build/scripts/installPackageMakefiles.py --package samurai
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
  make -j 8 install_include
  make -j 8 opt
  make -j 8 install
```

#### (c) Build and install the applications

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


