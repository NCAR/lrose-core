# The LROSE Makefile System

## Introduction

LROSE depende on ```make``` and the associated ```Makfiles```.

On Unix-type systems (LINUX, OSX) running the compiler is most commonly managed by the ```make``` application.
 
```make``` uses configuration files to decide what to do. These are either named:

  ```Makefile```

or

  ```makefile```


If both ```Makefile``` and ```makefile``` are present, the lower-case version takes precedence.

## What is a Makefile?

Makefile

[Makefile Tutorial](https://www.tutorialspoint.com/makefile/makefile_macros.htm)


LROSE has the following package options:

| Package       | Comments      |
| ------------- |:-------------:|
| lrose         | standard full package - the default |
| lrose-blaze   | blaze release - tested and documented |
| radx          | Radx apps only |
| titan         | Titan distribution |
| cidd          | CIDD display apps only, 32-bit build |

`lrose` is the standard build, which includes all of the libraries and applications in lrose, except for the `cidd` display and its related applications.

`lrose-blaze` is the first of the offical releases from the NSF SI2 LROSE project.

`radx` is a sub package that only includes the `Radx` applications.

`titan` is a sub package that supercedes the old Titan distribution for applications.

`cidd` is a special package, that must be built using 32-bit emulation, because the applications are based on the `xview` library that has no 64-bit port. This package includes the CIDD display, and other applications that depend on `xview`.

### Options for building LROSE on LINUX

There are three ways to build LROSE:

1. Check out the source from GitHub, and use AUTOMAKE and CONFIGURE for the build.
This is the standard approach.
See [README_AUTOMAKE_BUILD.md](./README_AUTOMAKE_BUILD.md) for details

2. Check out the source from GitHub, and use the NCAR build system.
This is recommended if you are actively involved in developing the code.
See [README_NCAR_BUILD.md](./README_NCAR_BUILD.md) for details

3. Download a pre-configured source distribution, and build from that.
See [README_DOWNLOAD_BUILD.md](./README_DOWNLOAD_BUILD.md) for details

### Building CIDD on LINUX

To build CIDD, see:
[README_CIDD.md](./README_CIDD.md).

### Building LROSE on MAC OSX

See:
[README_OSX_BUILD.md](./README_OSX_BUILD.md).

### Supported operating systems

LROSE was developed and tested extensively under LINUX.

Therefore LINUX is the preferred operating system.

However, LROSE can be compiled and run under Mac OSX.

Windows is supported using a Docker container, or the [Windows Subsystem For Linux](https://docs.microsoft.com/en-us/windows/wsl/install-win10)

### Required LINUX and gcc/g++ versions for LROSE build

Most good, up-to date LINUX distributions should work.

Recommended distributions are:

  * Debian
  * Ubuntu
  * RedHat
  * Centos
  * Fedora

### gcc/g++ versions for LROSE build

LROSE expects support for the c++11 standard.

The gcc/g++ version should be 4.8.5 or later.

### Required LINUX packages for the LROSE build

For a full LROSE build under LINUX, you need the following packages:

```
  epel-release
  
  tcsh
  perl
  perl-Env

  ftp
  git
  emacs
  tkcvs

  m4
  gcc
  g++
  gfortran

  glibc-devel
  libX11-devel
  libXext-devel (if available)
  libpng-devel
  libtiff-devel
  jasper-devel
  zlib-devel
  libexpat-devel
  libcurl-devel
  flex-devel
  fftw3-devel
  bzip2-devel
  qt4-devel

  gnuplot
  ImageMagick-devel
  ImageMagick-c++-devel

  xrdb
  Xvfb (virtual X server), specifically xorg-x11-server-Xvfb
  sshd (ssh logins)

  xorg-x11-fonts-misc
  xorg-x11-fonts-75dpi
  xorg-x11-fonts-100dpi
```

On Redhat-based hosts you can achieve this by running:

```
yum install -y epel-release
yum install -y \
tcsh perl perl-Env ftp git svn cvs tkcvs emacs tkcvs m4 \
gcc gcc-c++ gcc-gfortran glibc-devel libX11-devel libXext-devel \
libpng-devel libtiff-devel jasper-devel zlib-devel expat-devel libcurl-devel \
flex-devel fftw3-devel bzip2-devel jasper-devel \
qt5-qtbase-devel qt5-qtdeclarative-devel xrdb \
Xvfb xorg-x11-fonts-misc xorg-x11-fonts-75dpi xorg-x11-fonts-100dpi \
gnuplot ImageMagick-devel ImageMagick-c++-devel
```

On Debian-based hosts you can install the packages required to build the lrose-blaze formula with these commands:
(The debian packages don't install qmake-qt5. I worked around the problem with a symbolic link)

```
sudo apt-get update 
sudo apt-get install -y  \
    libbz2-dev libx11-dev libpng-dev libfftw3-dev \
    libjasper-dev qtbase5-dev qtdeclarative5-dev git \
    gcc g++ gfortran libfl-dev \
    automake make libtool pkg-config libexpat1-dev python

cd /usr/bin
ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake
ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake-qt5

```

### Required LINUX packages for the CIDD build

See: [README_CIDD.md](./README_CIDD.md).

### Running LROSE server-based applications

If you are making use of the data server applications in LROSE, you will need
to disable the firewall on your host, or open up the ports between 5300 and 5500.

To disable the firewall on a RedHat-based host:

```
  systemctl stop firewalld
  systemctl stop iptables
```



