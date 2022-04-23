# Building LROSE manually - OSX

Setting up the manual build environment allows a user to
develop LROSE-specific code in an efficient environment.

It uses Makefiles that are simple to use, rather than the complex makefiles generated
by the GNU tools autoconf and automake.

1. [prepare](#prepare)
2. [download](#download)
3. [setenv](#setenv)
4. [build using python script](#build-using-script)
5. [build manually](#build-manually)

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
  brew install hdf5
  brew install netcdf
  brew install udunits
  brew install fftw
  brew install flex
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

### Clone the current lrose-core version from GitHub

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

The LROSE manual build uses a recursive makefile approach, using environment variables to identify the various directories to be used during the build.

For details on the makefile system, see:

* [LROSE_manual_make_system.md](./LROSE_manual_make_system.md)

### Set environment variables for the build:

For sh or bash:
```
export HOST_OS=OSX_LROSE
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

LROSE will be installed in:
```
  $LROSE_INSTALL_DIR/bin
  $LROSE_INSTALL_DIR/lib
  $LROSE_INSTALL_DIR/include
```

Note: if there are errors related to Qt, try setting this variable
```
 export PKG_CONFIG_PATH="/usr/local/opt/qt/lib/pkgconfig"
 ```
 
<a name="build-using-script"/>

## 4. Build using python script provided

You can choose to build manually from the command line, or you
can use the python script provided.

The script ```build_lrose_manual.py``` is in ```lrose-core/build/scripts```.

The usage is:
```
  build_lrose_manual.py --help
  Usage: build_lrose_manual.py [options]
  Options:
    -h, --help         show this help message and exit
    --debug            Set debugging on
    --verbose          Set verbose debugging on
    --package=PACKAGE  Package name. Options are: lrose-core (default), lrose-
                       blaze, lrose-cyclone, lrose-cidd
    --prefix=PREFIX    Prefix name for install location
    --scripts          Install scripts as well as binaries

```

If you run using the defaults, it will install the lrose-core package into $HOME/lrose.

If you want to install the real-time runtime scripts, use ```--scripts```.


<a name="build-manually"/>

## 5. Build manually

### First install the makefiles if not building lrose-core

The `make` application can use makefiles named either `Makefile` or `makefile`.
The lower-case version takes preference.

The codebase contains upper-case Makefiles appropriate for building **lrose-core**.
If you are building lrose-core, you do not need this step.

If you are building a different version - for example **samurai**, you will need to install the makefiles for that version.

For the **samurai** distribtion, run the following:

```
  cd $LROSE_CORE_DIR
  ./build/scripts/installPackageMakefiles.py --package samurai
```

For the **lrose-radx** distribtion, run the following:

```
  cd $LROSE_CORE_DIR
  ./build/scripts/installPackageMakefiles.py --package lrose-radx
```

To re-install the **lrose-core** standard package Makefiles, perform the following:

```
  cd $LROSE_CORE_DIR
  ./build/scripts/installPackageMakefiles.py
```

since lrose-core is the default package.

### Perform the build

#### (a) Build and install the TDRP parameter handling utility

```
  cd $LROSE_CORE_DIR/codebase/libs/tdrp/src
  make install
  cd $LROSE_CORE_DIR/codebase/apps/tdrp/src/tdrp_gen
  make install
```

#### (b) Build and install the libraries

```
  cd $LROSE_CORE_DIR/codebase/libs/
  make -j 8 install_include
  make -j 8 install
```

If you get an install error, try:

```
  make install
```

#### (c) Build and install the applications

```
  cd $LROSE_CORE_DIR/codebase/apps
  make -j 8 install
```

If you get an install error, try:

```
  make install
```

#### (d) Install runtime scripts if needed

```
  cd $LROSE_CORE_DIR/codebase/apps/scripts/src
  make install
  cd $LROSE_CORE_DIR/codebase/apps/procmap/src/scripts
  make install
```

### Building individual applications

Once you have set up the environment specified above, you are free
to edit and build individual applications.

For example, if you want to work on RadxConvert, you would go
to the relevant directory and perform the build locally there.

```
  cd $LROSE_CORE_DIR/codebase/apps/Radx/src/RadxConvert
  make clean
  make
  make install
```

### Summary command list

The following is the full list of commands to run, in order.

You can cut-and-paste these into a terminal window to run the full procedure.

```
  cd $LROSE_CORE_DIR
  ./build/scripts/installPackageMakefiles.py
  cd $LROSE_CORE_DIR/codebase/libs/tdrp/src
  make install
  cd $LROSE_CORE_DIR/codebase/apps/tdrp/src/tdrp_gen
  make install
  cd $LROSE_CORE_DIR/codebase/libs/
  make -j 8 install_include
  make -j 8 install
  make install
  cd $LROSE_CORE_DIR/codebase/apps
  make -j 8 install
  make install
  cd $LROSE_CORE_DIR/codebase/apps/scripts/src
  make install
  cd $LROSE_CORE_DIR/codebase/apps/procmap/src/scripts
  make install
```


