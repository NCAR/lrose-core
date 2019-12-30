# Clone LROSE from GitHub and build from source - LINUX

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

### Clone the current version in GitHub

```
  git clone https://github.com/ncar/lrose-core 
```

The distribution will be in the lrose-core subdirectory:

```
  cd ~/git/lrose-core
```

<a name="build"/>

## 3. Build

### Run the build script:

```
  cd ~/git/lrose-core/build
```

To see the usage:

```
  ./checkout_and_build_auto.py --help
```

```
  Usage: checkout_and_build_auto.py [options]
  Options:
    -h, --help           show this help message and exit
    --clean              Cleanup tmp build dir
    --debug              Set debugging on
    --verbose            Set verbose debugging on
    --package=PACKAGE    Package name. Options are: lrose (default), cidd, radx,
                         titan, lrose-blaze
    --prefix=PREFIX      Install directory
    --buildDir=BUILDDIR  Temporary build dir
    --static             use static linking, default is dynamic
    --scripts            Install scripts as well as binaries
```

`package` defaults to `lrose`

`prefix` defaults to `${HOME}/lrose`

Available packages are:

```
  lrose lrose-blaze radx titan cidd
```

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
  ./checkout_and_build_auto.py --package radx
```

To set the install directory:

```
  ./checkout_and_build_auto.py --prefix /my/install/dir
```

To install the run-time scripts as well:

```
  ./checkout_and_build_auto.py --scripts
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


