# Build LROSE automatically using cmake - LINUX and MAC

1. [overview](#overview)
2. [prepare](#prepare)
3. [build](#build)
4. [verify](#verify)

<a name="overview"/>

## 1. Overview

Here we document the most autonomous way to perform the build.

We do this by checking out the build scripts from the bootstrap repository.

The lrose-core is checked out and built in /tmp.

Only the libraries and binaries are installed permanently.

The source code is in /tmp and is not permament.

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

## 3. Build automatically using scripts in the bootstrap repository

### Clone the bootstrap for LROSE

```
  cd ~/git
  git clone https://github.com/ncar/lrose-bootstrap
```

The distribution will be in the lrose-bootstrap subdirectory:

```
  cd ~/git/lrose-bootstrap/scripts
```

You will see the following scripts:

```
  do_build_all_cmake.py
  install_linux_packages.py
  lrose_checkout_and_build_cmake.py
```

```do_build_all_cmake.py``` calls ```install_linux_packages.py``` to ensure that all of the required packages are installed, prior to performing the compile. The install script determines the version of LINUX you are running and installs the appropriate dependency packages.

```do_build_all_cmake.py``` then calls ```lrose_checkout_and_build_cmake.py``` to checkout, compile and install lrose-core.

And finally ```do_build_all_cmake.py``` will optionally checkout, compile and install the wind applications ```fractl```, ```vortrac``` and ```samurai```.

### Run the ```do_build_all_cmake.py``` script:

To see the usage:

```
  cd ~/git/lrose-bootstrap/scripts
  ./do_build_all_cmake.py --help
```

```
Usage: do_build_all_cmake.py [options]

Options:
  -h, --help            show this help message and exit
  --clean               Cleanup tmp build dir
  --debug               Set debugging on
  --verbose             Set verbose debugging on
  --package=PACKAGE     Package name. Options are: lrose-core (default),
                        lrose-radx, lrose-cidd, apar, samurai
  --releaseDate=RELEASEDATE
                        Date from which to compute tag for git clone. Applies
                        if --tag is not used.
  --tag=TAG             Tag to check out lrose. Overrides --releaseDate
  --prefix=PREFIX       Install directory, default: /home/mdtest/lrose
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
  --buildNetcdf         Build netcdf and hdf5 from source
  --netcdfPrefix=NETCDFPREFIX
                        Netcdf install directory, default: /home/mdtest/lrose
  --fractl              Checkout and build fractl after core build is complete
  --vortrac             Checkout and build vortrac after core build is
                        complete
  --samurai             Checkout and build samurai after core build is
                        complete
  --cmake3              Use cmake3 instead of cmake
  --noApps              Do not build the lrose core apps
  --withJasper          Set if jasper library is installed. This provides
                        support for jpeg compression in grib files.
  --verboseMake         Verbose output for make, default is summary
  --iscray              True if the Cray compiler is used
  --isfujitsu           True if the Fujitsu compiler is used
```

For most cases, the defaults work well.

For MAC OSX, static builds are forced.

`package` defaults to `lrose-core`.

Available packages are:

```
  lrose-core lrose-cidd lrose-radx samurai
```

We recommend building the full core, unless you have specific limitations.

For older systems, such as centos7, you will need to use ```--cmake3```.

To build and install the full lrose package into ```~/lrose```:

```
  ./do_build_all_cmake.py
```

To specify the sub-package, e.g. lrose-radx:

```
  ./do_build_all_cmake.py --package lrose-radx
```

To set the install directory:

```
  ./do_build_all_cmake.py --prefix /my/install/dir
```

To cleanup between builds:

```
  ./do_build_all_cmake.py --clean
```

<a name="build-using-script"/>

<a name="verify"/>

## 4. Verify

As the build proceeds, the compile details will be saved to log files. The paths to these files are printed on in the terminal.

If the build fails, check the log files.

To see the build progress, use the ```--verbose``` option.

To see the full compile commands, use the ```--verbose``` and ```--verboseMake``` options.

If the build succeeds, test out some of the executables. Try the commands:

```
  ~/lrose/bin/RadxPrint -h
  ~/lrose/bin/RadxConvert -h
  ~/lrose/bin/Radx2Grid -h
  ~/lrose/bin/HawkEye
```

