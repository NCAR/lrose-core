# Checkout and build LROSE from source - using cmake - LINUX

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
  ./checkout_and_build_cmake.py --help
```

```
  Usage: checkout_and_build_cmake.py [options]
  Options:
    -h, --help            show this help message and exit
    --clean               Cleanup tmp build dir
    --debug               Set debugging on
    --verbose             Set verbose debugging on
    --package=PACKAGE     Package name. Options are: lrose-core (default),
                          lrose-radx, lrose-cidd, samurai
    --releaseDate=RELEASEDATE
                          Date from which to compute tag for git clone. Applies
                          if --tag is not used.
    --tag=TAG             Tag to check out lrose. Overrides --releaseDate
    --prefix=PREFIX       Install directory, default: /home/mdtest/lrose-install
    --buildDir=BUILDDIR   Temporary build dir, default: /tmp/lrose-build
    --logDir=LOGDIR       Logging dir, default: /tmp/lrose-build/logs
    --static              use static linking, default is dynamic
    --installAllRuntimeLibs
                          Install dynamic runtime libraries for all binaries, in
                          a directory relative to the bin dir. System libraries
                          are included.
    --installLroseRuntimeLibs
                          Install dynamic runtime lrose libraries for all
                          binaries, in a directory relative to the bin dir.
                          System libraries are not included.
    --noScripts           Do not install runtime scripts as well as binaries
    --buildNetcdf         Build netcdf and hdf5 from source
    --fractl              Checkout and build fractl after core build is complete
    --vortrac             Checkout and build vortrac after core build is
                          complete
    --samurai             Checkout and build samurai after core build is
                          complete
    --cmake3              Use cmake3 instead of cmake for samurai
    --no_core_apps        Do not build the lrose core apps
    --withJasper          Set if jasper library is installed. This provides
                          support for jpeg compression in grib files.
    --verboseMake         Verbose output for make, default is summary
```

`package` defaults to `lrose-core`

`prefix` defaults to `${HOME}/lrose`

Available packages are:

```
  lrose-core lrose-cidd lrose-radx
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
  ./checkout_and_build_cmake.py
```

To specify the sub-package, e.g.radx:

```
  ./checkout_and_build_cmake.py --package lrose-radx
```

To set the install directory:

```
  ./checkout_and_build_cmake.py --prefix /my/install/dir
```

To cleanup between builds:

```
  ./checkout_and_build_cmake.py --clean
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


