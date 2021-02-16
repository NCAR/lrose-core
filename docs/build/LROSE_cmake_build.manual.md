# Build LROSE manually using cmake - LINUX and MAC

1. [overview](#overview)
2. [prepare](#prepare)
3. [build](#build)
4. [verify](#verify)

<a name="overview"/>

## 1. Overview

This documents the manual build of lrose-core, using cmake.

This is the most hands-on cmake method.

It allows you to see all of the actions as they are carried out.

The default install prefix is ```~/lrose```.

The default directories for installation are:

```
  ~/lrose/include
  ~/lrose/lib
  ~/lrose/bin
```

This can be changed using the ```--prefix``` argument on the scripts.

<a name="prepare"/>

## 2. Prepare

### Create a working directory for cloning:

```
  mkdir -p ~/git
  cd ~/git
```

### Install required packages

See:

* [lrose-core package dependencies](./lrose_package_dependencies.md)

<a name="build"/>

## 3. Build manually

### Clone lrose-core

```
  cd ~/git
  git clone https://github.com/ncar/lrose-core
```

### Install the required package makefiles

This step is not needed if you are building the full core.

```
  cd ~/git/lrose-core/build/scripts
  ./installPackageMakefiles.py --help
```

The usage is:

```
  Usage: installPackageMakefiles.py [options]
  Options:
    -h, --help         show this help message and exit
    --debug            Set debugging on
    --package=PACKAGE  Package name.
                       lrose-core (default), lrose-radx, lrose-cidd, samurai
```

Select the package you want with ```--package```.

### Generate the CMakeLists.txt files for cmake

This step is not needed if you are building the full core, and do not want extra debugging.

```
  cd ~/git/lrose-core/build/cmake
  ./createCMakeLists.py --help
```

The usage is:

```
  Usage: createCMakeLists.py [options]
  Options:
    -h, --help            show this help message and exit
    --debug               Set debugging on
    --verbose             Set verbose debugging on
    --silent              Set debugging off
    --coreDir=COREDIR     Path of lrose-core top level directory, default is:
                          /data/mdtest/git/lrose-core/build/cmake/../..
    --prefix=PREFIX
                          Path of lrose install dir, default is ~/lrose
    --static              Create static lib objects. Default is shared
    --verboseMake         Verbose output for make, default is summary
    --withJasper          Set if jasper library is installed. This provides
                          support for jpeg compression in grib files.
```

To install in ```~/lrose```:

```
  cd ~/git/lrose-core/build/cmake
  ./createCMakeLists.py
```

To set the install directory:

```
  ./createCMakeLists.py --prefix /my/install/dir
```

### Make build directory and run cmake

```
  cd ~/git/lrose-core/codebase
  mkdir build
  cd build
  cmake ..
```

### Perform the build

Note: we have to build ```tdrp_gen``` before the apps, since it is a dependency.

Run the following commands:

```
  cd ~/git/lrose-core/codebase/build/libs
  make -j 8 install
  cd ~/git/lrose-core/codebase/build/apps/tdrp/src/tdrp_gen
  make -j 8 install
  cd ~/git/lrose-core/codebase/build/apps
  make -j 8 install
```

<a name="verify"/>

## 4. Verify

Try the commands:
```
  ~/lrose/bin/RadxPrint -h
  ~/lrose/bin/RadxConvert -h
  ~/lrose/bin/Radx2Grid -h
  ~/lrose/bin/HawkEye
```

