## Building CIDD using AUTOMAKE and CONFIGURE

### Choose your install directory (prefix)

The CIDD display uses the xview library, which must be compiled with 32-bit emulation.

**It is important** to keep the **cidd** build/install separate from the main build,
so that you do not mix 32-bit object files with 64-bit executables.

So, for a CIDD build we recommend you use a temporary prefix location, for example:

  `/tmp/cidd_m32`

After the package has been built into this temporary location, it can be copied into the final location.

### Check out, build and install netcdf support

Build and install netcdf into the temporary build area:

```
  git clone https://github.com/NCAR/lrose-netcdf
  cd lrose-netcdf
  ./build_and_install_netcdf.m32 -x /tmp/cidd_m32
```

This will install in `/tmp/cidd_m32`

### Check out LROSE

```
  git clone https://github.com/NCAR/lrose-core
```

<!---
### Install the makefile tree

The `make` application can use files named either `Makefile` or `makefile`.

The lower-case version takes preference.

The codebase contains, by default, upper-case Makefiles throughout the tree. These are **NOT** appropriate for the cidd build.

To get the correct build, you must install the lower-case makefiles relevant to the package you want to build.

To install the makefiles for the **cidd** package, perform the following:

```
  ./make_bin/install_package_makefiles.py --package cidd
```
--->

### Perform the build

Build using automake:

```
  cd lrose-core
  ./build/build_auto -p cidd -x /tmp/cidd_m32
```

### Copy the binaries to the final install location

Use rsync to copy the binaries to the final location.

For example:

```
  rsync -av /tmp/cidd_m32/bin /usr/local/lrose
```

The final install will be in:

```
  /usr/local/lrose/bin
```

### Required LINUX packages for the CIDD build

For a full CIDD build under LINUX, you need the following packages:

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

For 32-bit compatibility, install the following packages:

```
  glibc-devel.i686
  libX11-devel.i686
  libXext-devel.i686
  libjpeg-devel.i686
  libpng-devel.i686
  libstdc++-devel.i686
  libtiff-devel.i686

```

On Redhat-based hosts you can achieve this by running:

```
yum install -y glibc-devel.i686 libX11-devel.i686 libXext-devel.i686 \
libjpeg-devel.i686 libpng-devel.i686 libstdc++-devel.i686 \
libtiff-devel.i686
```

On Debian, you need to run the following:

```
  /usr/bin/dpkg --add-architecture i386
```

and use apt-get to install the following:

```
  libstdc++5:i386
  libstdc++6:i386
  libxml2:i386
  libgtk2.0-0:i386
  libgdk-pixbuf2.0-0:i386
```


