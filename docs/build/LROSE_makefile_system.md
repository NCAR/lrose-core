# The LROSE Makefile System

## Introduction

LROSE depends on ```make``` and the associated ```Makefiles```.

On Unix-type systems (LINUX, OSX) running the compiler is most commonly managed by the ```make``` application.
 
```make``` uses configuration files to decide what to do. These are named one of the following:

* ```Makefile```
* ```makefile```

If both ```Makefile``` and ```makefile``` are present, the lower-case version takes precedence.

In LROSE, ```Makefile``` is the primary name, and these files are checked in permanently in git. Various procedures you can run will cause a ```makefile``` to be written to a directory, which will then override the ```Makefile```.

## Anatomy of an LROSE Makefile

### Makefile elements

A Makefile contains several types of information:

  * macros - these store values that are used elsewhere in the file
  * targets - what is to be built
  * rules - on how to build targets
  * suffix rules - automatic rules depending in the suffix of a file

### Makefile tutorials

There are many tutorials on-line for Makefiles. For example see:

* [Makefile Tutorial](https://www.tutorialspoint.com/makefile/makefile_macros.htm)

### LROSE Makefile includes

The LROSE Makefiles are relatively short. The complexity is added by including partial makefiles, each with a specific purpose.

These partial makefiles reside in the directory:

* [lrose-core/build/make_include](../../build/make_include)

The following table lists the common top-level includes:

| Include file  | Purpose      |
| ------------- |:-------------:|
| [lrose_make_macros](../../build/make_include/lrose_make_macros) | main macro definitions |
| [lrose_make_targets](../../build/make_include/lrose_make_targets) | general target rules |
| [lrose_make_suffixes](../../build/make_include/lrose_make_suffixes) | suffix rules |
| [lrose_make_lib_targets](../../build/make_include/lrose_make_lib_targets) | targets for C libraries |
| [lrose_make_c_targets](../../build/make_include/lrose_make_c_targets) | targets for C apps |
| [lrose_make_c++_targets](../../build/make_include/lrose_make_c_targets) | targets for C++ apps |
| [lrose_make_qt_targets](../../build/make_include/lrose_make_qt_targets) | extra targets for QT apps |

### Macros for specific OS versions

In [lrose_make_macros](../../build/make_include/lrose_make_macros), you will see the following line:

```
include $(LROSE_CORE_DIR)/build/make_include/lrose_make.$(HOST_OS)
```

This includes a file that defines macros specific to the OS you are running.

For this to work, you need to set the **HOST_OS** environment variable.

The common OS versions supported, along with the include files, are listed in the following table:

| HOST_OS  | Include file       | Comments |
| ------------- |:-------------:|:--------:|
| [lrose_make.LINUX_LROSE](../../build/make_include/lrose_make.LINUX_LROSE) | normal LINUX build |
| [lrose_make.OSX_LROSE](../../build/make_include/lrose_make.OSX_LROSE) | build on Mac OSX |
| [lrose_make.CIDD_32](../../build/make_include/lrose_make.CIDD_32) | 32-bit build for CIDD on LINUX |

### LROSE Makefile templates

The actual Makefiles are created by filling out elements in a template. As mentioend above, the complexity is added by including partial makefiles.

The following template is for a library code subdirectory:

```
  # main macros
  include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros
  # local macros
  LOC_INCLUDES =
  LOC_CFLAGS =
  # target - library .a file
  TARGET_FILE =
  # list of headers
  HDRS =
  # list of C sources
  SRCS =
  # list of C++ sources
  CPPC_SRCS =
  # list of FORTRAN sources
  F_SRCS =
  # general targets
  include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets
```

For library code examples, see:

* [codebase/libs/dataport/src/bigend/Makefile](../../codebase/libs/dataport/src/bigend/Makefile)
* [codebase/libs/toolsa/src/pjg/Makefile](../../codebase/libs/toolsa/src/pjg/Makefile)
* [codebase/libs/Mdv/src/Mdvx/Makefile](../../codebase/libs/Mdv/src/Mdvx/Makefile)

The following template is for an application code directory:

```
  # include main macros
  include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros
  # main target - application name
  TARGET_FILE =
  # local macros
  LOC_INCLUDES =
  LOC_LIBS =
  LOC_LDFLAGS =
  LOC_CFLAGS =
  # header code files
  HDRS =
  # list of C sources
  SRCS =
  # list of C++ sources
  CPPC_SRCS =
  # list of FORTRAN sources
  F_SRCS =
  # tdrp macros
  include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros
  # C++ targets
  include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets
  # tdrp targets
  include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets
```

For application examples, see:

* [codebase/apps/Radx/src/RadxConvert/Makefile](../../codebase/apps/Radx/src/RadxConvert/Makefile)
* [codebase/apps/mdv_utils/src/PrintMdv/Makefile](../../codebase/apps/mdv_utils/src/PrintMdv/Makefile)
* [codebase/apps/radar/src/HawkEye/Makefile](../../codebase/apps/radar/src/HawkEye/Makefile)

The HawkEye example is more complicated, because it is a QT application, so we need to handle the QT complexities.

## Recursion through the code tree

LROSE sets up Makefiles at all levels of the code tree, both for the libraries and applications.
Except for the lowest level, where the actual code files reside, the Makefiles handle recursion to lower levels in the code tree.

As an example, for the dataport library, we have the following, from the top level to the bottom level:

* [codebase/libs/Makefile](../../codebase/libs/Makefile)
* [codebase/libs/dataport/Makefile](../../codebase/libs/dataport/Makefile)
* [codebase/libs/dataport/src/Makefile](../../codebase/libs/dataport/src/Makefile)
* [codebase/libs/dataport/src/bigend/Makefile](../../codebase/libs/dataport/src/bigend/Makefile)

Similarly, for the RadxConvert application, we have the following, from the top level to the bottom level:

* [codebase/apps/Makefile](../../codebase/apps/Makefile)
* [codebase/apps/Radx/Makefile](../../codebase/apps/Radx/Makefile)
* [codebase/apps/Radx/src/Makefile](../../codebase/apps/Radx/src/Makefile)
* [codebase/apps/Radx/src/RadxConvert/Makefile](../../codebase/apps/Radx/src/RadxConvert/Makefile)


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



