# LROSE-CIDD 32-bit package dependencies - LINUX ONLY

**NOTE:** the following procedures require **root** or **sudo** privileges.

## Auto-installing packages using python script

You can install the required packages using a python script provided by the lrose-core.
This script detects the OS version from the ```/etc/os-release``` file.
It then installs the packages appropriate for the OS version.

Check out lrose-core:

```
  git clone https://github.com/ncar/lrose-core
```

Then run:

```
  lrose-core/build/install_linux_packages.py --cidd32
```

## Manually install 32-bit packages for CENTOS/RHEL 6/7, and FEDORA

```
  yum install -y \
    xrdb \
    glibc-devel.i686 libX11-devel.i686 libXext-devel.i686 \
    libtiff-devel.i686 libpng-devel.i686 libcurl-devel.i686 \
    libstdc++-devel.i686 libtiff-devel.i686 \
    zlib-devel.i686 expat-devel.i686 flex-devel.i686 \
    fftw-devel.i686 bzip2-devel.i686 \
    gnuplot ImageMagick-devel ImageMagick-c++-devel \
    xorg-x11-fonts-100dpi xorg-x11-fonts-ISO8859-1-100dpi \
    xorg-x11-fonts-75dpi xorg-x11-fonts-ISO8859-1-75dpi \
    xorg-x11-fonts-misc
```

## Manually install packages for CENTOS/RHEL 8

```
  dnf install -y --allowerasing \
    xrdb \
    glibc-devel.i686 libX11-devel.i686 libXext-devel.i686 \
    libcurl-devel.i686 \
    libtiff-devel.i686 libpng-devel.i686 \
    libstdc++-devel.i686 libtiff-devel.i686 \
    zlib-devel.i686 expat-devel.i686 flex-devel.i686 \
    fftw-devel.i686 bzip2-devel.i686 \
    gnuplot ImageMagick-devel ImageMagick-c++-devel \
    xorg-x11-fonts-100dpi xorg-x11-fonts-ISO8859-1-100dpi \
    xorg-x11-fonts-75dpi xorg-x11-fonts-ISO8859-1-75dpi \
    xorg-x11-fonts-misc
```

## Manually install packages for CENTOS/RHEL 9

```
  dnf install -y --allowerasing \
    xrdb \
    glibc-devel.i686 libX11-devel.i686 libXext-devel.i686 \
    libcurl-devel.i686 \
    libtiff-devel.i686 libpng-devel.i686 \
    libstdc++-devel.i686 libtiff-devel.i686 \
    zlib-devel.i686 expat-devel.i686 \
    fftw-devel.i686 bzip2-devel.i686 \
    gnuplot ImageMagick-devel ImageMagick-c++-devel \
    xorg-x11-fonts-100dpi xorg-x11-fonts-ISO8859-1-100dpi \
    xorg-x11-fonts-75dpi xorg-x11-fonts-ISO8859-1-75dpi \
    xorg-x11-fonts-misc
```

## Manually install 32-bit packages on Debian and Ubuntu

```
  /usr/bin/dpkg --add-architecture i386
  apt-get -y update
   apt-get install -y \
   libx11-dev:i386 \
   libxext-dev:i386 \
   libfftw3-dev:i386 \
   libexpat-dev:i386 \
   libpng-dev:i386 \
   libfl-dev:i386 \
   libbz2-dev:i386 \
   libzip-dev:i386
```

## Manually install 32-bit packages on SUSE-based

Suse uses the same package lists as RedHat.

Use ```zypper``` instead of ```yum```, with the same package list as above.

```
  zypper install -y \
    xrdb \
    glibc-devel-32bit libX11-devel-32bit libXext-devel-32bit \
    libtiff-devel-32bit libpng-devel-32bit libcurl-devel-32bit \
    libstdc++-devel-32bit libtiff-devel-32bit \
    zlib-devel-32bit libexpat-devel-32bit flex-32bit \
    libfftw3-3-32bit libbz2-devel-32bit \
    gnuplot ImageMagick-devel-32bit \
    xorg-x11 xorg-x11-devel xorg-x11-fonts xorg-x11-fonts-core
```

## 64-bit packages for lrose-core

See:

* [lrose_package_dependencies.cidd.md](./lrose_package_dependencies.md)

