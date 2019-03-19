# Download LROSE release and build from source - LINUX

1. [prepare](#prepare)
2. [download](#download)
3. [build](#build)
4. [verify](#verify)
5. [handling errors](#handling_errors)
6. [update](#update)

<a name="prepare"/>

## 1. Prepare

Most good, up-to date LINUX distributions should work.

Recommended distributions are:

  * Debian
  * Ubuntu (based on Debian)
  * RedHat
  * Centos (based on RedHat)
  * Fedora (based on RedHat)

First, you will need to install the required packages.

### On Redhat-based hosts, run the following (as root or sudo):

```
sudo yum install -y epel-release

sudo yum install -y \
    tcsh wget git \
    tkcvs emacs rsync python \
    m4 make libtool autoconf automake \
    gcc gcc-c++ gcc-gfortran glibc-devel \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel zlib-devel \
    expat-devel libcurl-devel \
    flex-devel fftw3-devel \
    bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel \
    hdf5-devel netcdf-devel \
    xorg-x11-xauth xorg-x11-apps \
    rpm-build redhat-rpm-config \
    rpm-devel rpmdevtools

cd /usr/bin
sudo ln -s qmake-qt5 qmake
```

### On Debian-based hosts run the following (as root or sudo):

```
sudo apt-get update 

sudo apt-get install -y \
    git gcc g++ gfortran cmake rsync mlocate \
    automake make libtool pkg-config python \
    libcurl3-dev curl \
    libfl-dev libbz2-dev libx11-dev libpng-dev \
    libfftw3-dev libexpat1-dev \
    qtbase5-dev qtdeclarative5-dev \
    libgeographic-dev libeigen3-dev libzip-dev \
    libnetcdf-dev netcdf-bin libhdf5-dev hdf5-tools

# create link for qtmake

cd /usr/bin
sudo /bin/rm -f qmake qmake-qt5
sudo ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake
sudo ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake-qt5
```

<a name="download"/>

## 2. Download

### Create a working directory for building the distribution:

```
  mkdir ~/lrose_build
  cd ~/lrose_build
```

### Download the source release for Linux

Download the source tar file from:

```
  https://github.com/NCAR/lrose-core/releases 
```

A typical source release would be:

```
  lrose-20160823.src.tgz
```

### Untar

```
  tar xvfz lrose-20160823.src.tgz
```

The distribution will be unpacked into a subdirectory:

```
  cd ~/lrose_build/lrose-20160823.src
```

<a name="build"/>

## 3. Build

### Run the build script:

To see the usage:

```
  ./build_src_release.py --help
```

To build and install into the default directory: `~/lrose`

```
  ./build_src_release.py
```

Or set an install directory:

```
  ./build_src_release.py --prefix /my/install/dir
```

<a name="verify"/>

## 4. Verify

Look in ~/lrose or /my/install/dir for

```
  include
  lib
  bin
```

Try the commands:
```
  ~/lrose/bin/RadxPrint -h
  ~/lrose/bin/RadxConvert -h
  ~/lrose/bin/Radx2Grid -h
  ~/lrose/bin/HawkEye
```

<a name="handling_errors"/>

## 5. Handling build errors

If the build does not complete successfully, you will need to
track down the errors.

The very first errors in the build are the most important.

If you get errors, go into the directory giving problems, and
run the make as follows:

```
  make |& less
```

and scroll down searchinf for `error`.

Alternatively, run

```
  make >& make.log
```

and then inspect the make.log file, searching for `error`.

<a name="update"/>

## 6. Update

To update, just repeat the procedure in sections 2, 3 and 4 above.

The new version will be installed over the previous version.


