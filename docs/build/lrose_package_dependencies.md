# LROSE package dependencies

LROSE depends on a large number of LINUX packages.

The following are the commands for installing those packages.

## RedHat-based systems

These apply to:

  * RedHat 6 and 7
  * CentOS 6 and 7 (based on RedHat)
  * Fedora (based on RedHat)

```
  yum install -y epel-release

  yum install -y \
    tcsh wget git \
    tkcvs emacs rsync python mlocate \
    m4 make cmake libtool autoconf automake \
    gcc gcc-c++ gcc-gfortran glibc-devel \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel zlib-devel libzip-devel \
    GeographicLib-devel eigen3-devel armadillo-devel \
    expat-devel libcurl-devel openmpi-devel \
    flex-devel fftw3-devel \
    bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel \
    hdf5-devel netcdf-devel \
    xorg-x11-xauth xorg-x11-apps \
    rpm-build redhat-rpm-config \
    rpm-devel rpmdevtools
```

## CENTOS 8 and RHEL 8

```
  dnf install -y epel-release
  dnf install -y 'dnf-command(config-manager)'
  dnf config-manager --set-enabled PowerTools
  dnf install -y python3
  dnf install -y python2-devel platform-python-devel
  dnf install -y \
    tcsh wget git \
    emacs rsync python2 python3 mlocate \
    m4 make cmake libtool autoconf automake \
    gcc gcc-c++ gcc-gfortran glibc-devel \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel zlib-devel libzip-devel \
    eigen3-devel armadillo-devel \
    expat-devel libcurl-devel openmpi-devel \
    flex-devel fftw3-devel \
    bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel \
    hdf5-devel netcdf-devel \
    xorg-x11-xauth xorg-x11-apps \
    rpm-build redhat-rpm-config \
    rpm-devel rpmdevtools
  alternatives --set python /usr/bin/python3
```

The following are missing from the CENTOS 8 install:

```
    tkcvs
    GeographicLib-devel
```

## SUSE-based systems

Suse uses the same package lists as RedHat.

Use ```zypper``` instead of ```yum```, with the same package list as above.

## Debian-based systems

These apply to:

  * Debian
  * Ubuntu (based on Debian)

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
    libarmadillo-dev libopenmpi.dev \
    libnetcdf-dev netcdf-bin libhdf5-dev hdf5-tools

# create link for qmake

cd /usr/bin
sudo /bin/rm -f qmake qmake-qt5
sudo ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake
sudo ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake-qt5
```

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

