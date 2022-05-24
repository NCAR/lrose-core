# Checkout and build LROSE from source - using autoconf - LINUX

This will checkout and build the latest source from GitHub.

1. [prepare](#prepare)
2. [download](#download)
3. [build](#build)
4. [verify](#verify)

<a name="prepare"/>

## 1. Prepare

### Most good, up-to date LINUX distributions should work.

Recommended distributions are:

  * Debian
  * Ubuntu (based on Debian)
  * RedHat
  * CentOS (based on RedHat)
  * Fedora (based on RedHat)

First, you will need to install the required packages.

See: [LROSE package dependencies](../build//lrose_package_dependencies.md)

<a name="download"/>

## 2. Download from GitHub

Create a working directory for cloning:

```
  mkdir -p ~/git
  cd ~/git
```

### Clone the bootstrap for LROSE

```
  git clone https://github.com/ncar/lrose-bootstrap
```

The distribution will be in the lrose-bootstrap subdirectory:

```
  cd ~/git/lrose-bootstrap
```

<a name="build"/>

## 3. Build

### Run the build script:

```
  cd ~/git/lrose-bootstrap/scripts
```

To see the usage:

```
  ./checkout_and_build_auto.py --help
```

```
  Usage: checkout_and_build_auto.py [options]
  Options:
    -h, --help            show this help message and exit
    --clean               Cleanup tmp build dir
    --debug               Set debugging on
    --verbose             Set verbose debugging on
    --package=PACKAGE     Package name. Options are: lrose-core (default),
                          lrose-blaze, lrose-cyclone, lrose-radx, lrose-cidd
    --releaseDate=RELEASEDATE
                          Tag to check out lrose-core
    --prefix=PREFIX       Install directory, default is ~/lrose
    --buildDir=BUILDDIR   Temporary build dir, default is /tmp/lrose_build
    --logDir=LOGDIR       Logging dir, default is /tmp/lrose_build/logs
    --static              use static linking, default is dynamic
    --buildNetcdf         Build NetCDF and HDF5 instead of using the system libs
    --fractl              Checkout and build fractl after core build is complete
    --vortrac             Checkout and build vortrac after core build is complete
    --samurai             Checkout and build samurai after core build is complete
    --cmake3              Use cmake3 instead of cmake for samurai
    --geolib              Build and install geolib - for fractl, samurai

```

`package` defaults to `lrose-core`

`prefix` defaults to `${HOME}/lrose`

Available packages are:

```
  lrose-core lrose-cidd lrose-radx lrose-blaze lrose-cyclone
```

We recommend just building the full core.

The default directories for installation are:

```
  ~/lrose/include
  ~/lrose/lib
  ~/lrose/bin
```

To build and install the full lrose package into the default directory:

```
  ./checkout_and_build_auto.py
```

To specfiy the sub-package, e.g.radx:

```
  ./checkout_and_build_auto.py --package lrose-radx
```

To set the install directory:

```
  ./checkout_and_build_auto.py --prefix /my/install/dir
```

To cleanup between builds:

```
  ./checkout_and_build_auto.py --clean
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

## 5. Handling build errors

If the build does not complete successfully, you will need to
track down the errors.

The very first errors in the build are the most important.

If you get errors, go into the directory giving problems, and
run the make as follows:

```
  make |& less
```

and scroll down searching for `error`.

Alternatively, run

```
  make >& make.log
```

and then inspect the make.log file, searching for `error`.


