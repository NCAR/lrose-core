# LROSE package dependencies - CIDD

See also the following for the main packages:

* [lrose_package_dependencies.md](./lrose_package_dependencies.md)

# CIDD build - 32-bit packages required

For the CIDD display, we need to perform a 32-bit build.

## Redhat and CENTOS

On Redhat-based hosts you can achieve this by running:

```
yum install -y tcsh perl perl-Env ftp git svn cvs tkcvs emacs \
gcc gcc-c++ gcc-gfortran \
glibc-devel.i686 libX11-devel.i686 libXext-devel.i686 \
libtiff-devel.i686 libpng-devel.i686 libcurl-devel.i686 \
libstdc++-devel.i686 libtiff-devel.i686 \
zlib-devel.i686 expat-devel.i686 flex-devel.i686 \
fftw-devel.i686 bzip2-devel.i686 xrdb Xvfb \
gnuplot ImageMagick-devel ImageMagick-c++-devel \
xorg-x11-fonts-100dpi xorg-x11-fonts-ISO8859-1-100dpi \
xorg-x11-fonts-75dpi xorg-x11-fonts-ISO8859-1-75dpi \
xorg-x11-fonts-misc
```

## Debian-based systems

On Debian and Ubuntu, you need to run the following:

```
  /usr/bin/dpkg --add-architecture i386
  apt-get update
```

and use apt-get to install the following:

```
  apt-get install libx11-6:i386 \
                   libstdc++-4.9-dev:i386 \
                   libpng-dev:i386 \
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

