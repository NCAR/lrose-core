# Build LROSE using script to run cmake - LINUX and MAC

1. [overview](#overview)
2. [prepare](#prepare)
3. [build](#build)
4. [verify](#verify)

<a name="overview"/>

## 1. Overview

This documents building using a script in lrose-core.

The libraries and binaries are installed permanently.

The source code will reside in your home directory.

The default install prefix is ```~/lrose```.

The default directories for installation are:

```
  ~/lrose/include
  ~/lrose/lib
  ~/lrose/bin
```

This can be changed using the ```--prefix``` argument on the scripts.

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

## 3. Build using scripts in lrose-core

### Clone lrose-core

```
  cd ~/git
  git clone https://github.com/ncar/lrose-core
```

### Run the ```build_lrose_cmake.py``` script:

To see the usage:

```
  cd ~/git/lrose-core/build/scripts
  ./build_lrose_cmake.py --help
```

```
  Usage: build_lrose_cmake.py [options]
  Options:
    -h, --help         show this help message and exit
    --debug            Set debugging on
    --verbose          Set verbose debugging on
    --package=PACKAGE  Package name. Options are: lrose-core (default), lrose-
                       radx, lrose-cidd, samurai
    --prefix=PREFIX    Prefix name for install location
    --scripts          Install scripts as well as binaries
    --static           use static linking, default is dynamic
    --cmake3           Use cmake3 instead of cmake for samurai
```

`package` defaults to `lrose-core`.

Available packages are:

```
  lrose-core lrose-cidd lrose-radx samurai
```

We recommend building the full core.

For older systems, such as centos7, you will need to use ```--cmake3```.

To build and install the full lrose package into ```~/lrose```:

```
  ./build_lrose_cmake.py
```

To specify the sub-package, e.g. lrose-radx:

```
  ./build_lrose_cmake.py --package lrose-radx
```

To set the install directory:

```
  ./build_lrose_cmake.py --prefix /my/install/dir
```

<a name="build-manually"/>

<a name="verify"/>

## 4. Verify

Try the commands:
```
  ~/lrose/bin/RadxPrint -h
  ~/lrose/bin/RadxConvert -h
  ~/lrose/bin/Radx2Grid -h
  ~/lrose/bin/HawkEye
```

