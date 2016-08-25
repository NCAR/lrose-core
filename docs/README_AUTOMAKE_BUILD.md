# Building using AUTOMAKE and CONFIGURE

### Available package builds

LROSE has the following package options:

| Package       | Comments      |
| ------------- |:-------------:|
| lrose         | standard full build - the default |
| radx          | Radx apps only |
| hcr           | HCR (Hiaper Cloud Radar) build |
| cidd          | CIDD display apps only, 32-bit build |

If you want to build the `cidd` package, see:

  [README_CIDD_BUILD.md](./README_CIDD_BUILD.md)

### Choose your install directory (prefix)

The default is: `/usr/local/lrose`

### Check out, build and install netcdf support

The following default:

```
  git clone https://github.com/NCAR/lrose-netcdf
  cd lrose-netcdf
  ./build_and_install_netcdf
```

will build and install netcdf in `/usr/local/lrose`

For, say, installing in `/tmp/mybuild`:

```
  ./build_and_install_netcdf -x /tmp/mybuild
```

### Check out LROSE

```
  git clone https://github.com/NCAR/lrose-core
```

<!---
### Install the makefile tree

The `make` application can use files named either `Makefile` or `makefile`.

The lower-case version takes preference.

The codebase contains, by default, upper-case Makefiles throughout the tree. These are **NOT** appropriate for the build.

To get the correct build, you must install the lower-case makefiles relevant to the package you want to build.

To install the makefiles for the **lrose** standard package, perform the following:

```
  cd lrose-core/codebase
  ./make_bin/install_package_makefiles.py
```
This is equivalent to the following

```
  ./make_bin/install_package_makefiles.py --package lrose
```

If you want to perform a package-specific build, you can specify that on the command line.

As an example, for the **radx** distribtion, run the following:

```
  ./make_bin/install_package_makefiles.py --package radx
```

--->

### Perform the build

To build using automake:

```
  cd lrose-core
  ./build/build_auto -p package -x prefix
```

where `prefix` is the location into which you are building.

`package` defaults to `lrose`

`prefix` defaults to `/usr/local/lrose`


### Required LINUX and gcc/g++ versions for LROSE build

Most good, up-to date LINUX distributions should work.

Recommended distributions are:

  * Debian
  * Ubuntu
  * RedHat
  * Centos
  * Fedora

LROSE expects good support for the c++11 standard.

The gcc/g++ version should be 4.8.5 or later.

### Required LINUX packages for the LROSE build

For a full LROSE build under LINUX, you need the following packages:

```
  tcsh
  perl
  perl-Env

  ftp
  git
  svn
  cvs
  tkcvs
  emacs

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
  flex-devel
  fftw3-devel
  bzip2-devel
  jasper-devel
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
yum install -y tcsh perl perl-Env ftp git svn cvs tkcvs emacs \
gcc gcc-c++ gcc-gfortran glibc-devel libX11-devel libXext-devel \
libpng-devel libtiff-devel jasper-devel zlib-devel expat-devel \
flex-devel fftw3-devel bzip2-devel jasper-devel qt4-devel xrdb \
Xvfb xorg-x11-fonts-misc xorg-x11-fonts-75dpi xorg-x11-fonts-100dpi \
gnuplot ImageMagick-devel ImageMagick-c++-devel
```


