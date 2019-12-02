# LROSE package dependencies

LROSE depends on a large number of LINUX packages.

The following are the commands for installing those packages.

## RedHat-based systems

These apply to:

  * RedHat
  * CentOS (based on RedHat)
  * Fedora (based on RedHat)

For *RHEL/CentOS 8 only*, you first need to enable the PowerTools repository:

```
  yum config-manager --set-enabled PowerTools
```
For *all* RedHat-based distributions, you need to add the Extra Packages for Enterprise Linux (EPEL) repository and then install the packages required for LROSE:

```
sudo yum install -y epel-release

sudo yum install -y \
    tcsh wget git \
    tkcvs emacs rsync python \
    m4 make cmake cmake3 libtool autoconf automake \
    gcc gcc-c++ gcc-gfortran glibc-devel \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel zlib-devel libzip-devel \
    GeographicLib-devel eigen3-devel armadillo \
    expat-devel libcurl-devel \
    flex-devel fftw3-devel \
    bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel \
    hdf5-devel netcdf-devel \
    xorg-x11-xauth xorg-x11-apps \
    rpm-build redhat-rpm-config \
    rpm-devel rpmdevtools
```

For CENTOS 8 you need to add the PowerTools repo, and python packages:

```
  yum config-manager --set-enabled PowerTools
  yum install -y python2-devel platform-python-devel
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
    libnetcdf-dev netcdf-bin libhdf5-dev hdf5-tools

# create link for qmake

cd /usr/bin
sudo /bin/rm -f qmake qmake-qt5
sudo ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake
sudo ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake-qt5
```

