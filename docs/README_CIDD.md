## The CIDD build and run-time environment

CIDD must be built as a 32-bit application.

And to run CIDD, you must have the required 32-bit packages installed.

### Required LINUX packages for the CIDD build

For a full CIDD build under LINUX, you need the following packages:

```
  tcsh
  perl
  perl-Env

  ftp
  git

  gcc
  g++
  gfortran

  glibc-devel.i686
  libX11-devel.i686
  libXext-devel.i686 (if available)
  libtiff-devel.i686
  libpng-devel.i686
  libstdc++-devel.i686
  libtiff-devel.i686
  zlib-devel.i686
  expat-devel.i686
  flex-devel.i686
  fftw-devel.i686
  bzip2-devel.i686

  gnuplot
  ImageMagick-devel
  ImageMagick-c++-devel

  xrdb
  Xvfb (virtual X server), specifically xorg-x11-server-Xvfb
  sshd (ssh logins)

  xorg-x11-fonts-misc
  xorg-x11-fonts-75dpi
  xorg-x11-fonts-100dpi
  xorg-x11-fonts-ISO8859-1-100dpi
  xorg-x11-fonts-ISO8859-1-75dpi
```

On Redhat-based hosts you can achieve this by running:

```
yum install -y tcsh perl perl-Env ftp git svn cvs tkcvs emacs \
gcc gcc-c++ gcc-gfortran \
glibc-devel.i686 libX11-devel.i686 libXext-devel.i686 \
libtiff-devel.i686 libpng-devel.i686 \
libstdc++-devel.i686 libtiff-devel.i686 \
zlib-devel.i686 expat-devel.i686 flex-devel.i686 \
fftw-devel.i686 bzip2-devel.i686 xrdb Xvfb \
gnuplot ImageMagick-devel ImageMagick-c++-devel \
xorg-x11-fonts-100dpi xorg-x11-fonts-ISO8859-1-100dpi \
xorg-x11-fonts-75dpi xorg-x11-fonts-ISO8859-1-75dpi \
xorg-x11-fonts-misc
```

On Debian, you need to run the following:

```
  /usr/bin/dpkg --add-architecture i386
  apt-get update
```

and use apt-get to install the following:

```
  apt-get install libx11-6:i386 \
                   libstdc++-4.9-dev:i386 \
                   libpng12-dev:i386 \
                   libx11-dev:i386 \
                   libxext-dev:i386 \
                   lib32stdc++-4.9-dev \
                   xviewg:i386 xviewg-dev:i386 \
                   libstdc++5:i386 \
                   libstdc++6:i386 \
                   libxml2:i386 \
                   libgtk2.0-0:i386 \
                   libgdk-pixbuf2.0-0:i386 \
                   libbz2-dev:i386
```

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
  rsync -av /tmp/cidd_m32/bin ${HOME}/lrose
```

The final install will be in:

```
  ${HOME}/lrose/bin
```


