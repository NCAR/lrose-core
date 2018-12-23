## LROSE download and build from source - LINUX

1. [prepare](#prepare)
2. [install](#install)
3. [verify](#verify)

<a name="prepare"/>

### 1. Prepare

#### Required LINUX versions for LROSE build

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
libpng-devel libtiff-devel jasper-devel zlib-devel expat-devel \
flex-devel fftw3-devel bzip2-devel jasper-devel qt5-qtbase-devel xrdb \
Xvfb xorg-x11-fonts-misc xorg-x11-fonts-75dpi xorg-x11-fonts-100dpi \
gnuplot ImageMagick-devel ImageMagick-c++-devel

cd /usr/bin; sudo ln -s qmake-qt5 qmake

```

On Debian-based hosts run the following (as root or sudo):

```
sudo apt-get update 

sudo apt-get install -y  \
    libbz2-dev libx11-dev libpng12-dev libfftw3-dev \
    libjasper-dev qtbase5-dev git \
    gcc g++ gfortran libfl-dev \
    automake make libtool pkg-config libexpat1-dev python

cd /usr/bin; sudo ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake

```

<a name="install"/>

### 2. Download

Create a working directory for building the distribution:

```
  mkdir ~/lrose_build
  cd ~/lrose_build
```

#### Download the source release for Linux

Download the source tar file from:

```
  https://github.com/NCAR/lrose-core/releases 
```

A typical source release would be:

```
  lrose-blaze-20160823.src.tgz
```

#### Untar

```
  tar xvfz lrose-blaze-20160823.src.tgz
```

The distribution will be unpacked into a subdirectory:

```
  ~/lrose_build/lrose-blaze-20160823.src
```

#### Run the build script:

Build and install into the default directory: `~/lrose`

```
  ./build_src_release.py
```
or set an install directory:

```
  ./build_src_release.py --prefix /my/install/dir
```

<a name="verify"/>

### 3. Verify

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

### Handling build errors

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


