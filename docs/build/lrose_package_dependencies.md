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

