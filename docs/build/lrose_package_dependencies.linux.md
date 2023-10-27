# LROSE-CORE package dependencies - LINUX

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
  lrose-core/build/install_linux_packages.py
```

## Manually install packages for CENTOS-6, RHEL-6

This requires upgrading the compiler toolchain.

```
  yum install -y epel-release

  yum install -y tcsh wget git tkcvs \
    emacs rsync python \
    m4 make cmake libtool autoconf automake \
    gcc gcc-c++ gcc-gfortran glibc-devel \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel zlib-devel \
    expat-devel libcurl-devel \
    flex-devel fftw3-devel \
    bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel \
    hdf5-devel netcdf-devel \
    xorg-x11-xauth xorg-x11-apps \
    gnuplot ImageMagick-devel ImageMagick-c++-devel \
    rpm-build redhat-rpm-config \
    rpm-devel rpmdevtools

  # create link for qtmake

  cd /usr/bin; \
  ln -s qmake-qt5 qmake;

  # install updated gcc and g++ toolchain

    wget http://people.centos.org/tru/devtools-2/devtools-2.repo -O /etc/yum.repos.d/devtools-2.repo; \
    yum install -y devtoolset-2-gcc devtoolset-2-binutils; \
    yum install -y devtoolset-2-gcc-c++ devtoolset-2-gcc-gfortran

  # copy the updated compilers into /usr
  # so that they become the system default

  cd /opt/rh/devtoolset-2/root; \
  rsync -av usr /
```

## Manually install packages for CENTOS-7, RHEL-7

```
  yum install -y epel-release

  yum install -y \
    tcsh wget git \
    tkcvs emacs rsync python mlocate \
    m4 make cmake cmake3 libtool autoconf automake \
    gcc gcc-c++ gcc-gfortran glibc-devel \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel zlib-devel libzip-devel \
    eigen3-devel armadillo-devel \
    expat-devel libcurl-devel openmpi-devel \
    flex-devel fftw3-devel \
    bzip2-devel qt6-qtbase-devel qt6-qtdeclarative-devel \
    hdf5-devel netcdf-devel \
    xorg-x11-xauth xorg-x11-apps \
    gnuplot ImageMagick-devel ImageMagick-c++-devel \
    rpm-build redhat-rpm-config \
    rpm-devel rpmdevtools

  # create link for qtmake

  cd /usr/bin;
  ln -s qmake-qt6 qmake;
```

## Manually install packages for RHEL 8, ALMA LINUX 8, ROCKY 8

```
  dnf install -y epel-release ; \
  dnf install -y 'dnf-command(config-manager)' ; \
  dnf config-manager --set-enabled powertools ; \
  dnf install -y python3 ; \
  dnf install -y python2-devel platform-python-devel ; \
  dnf install -y \
    tcsh wget git tk-devel perl-Env \
    emacs rsync python2 python3 mlocate \
    m4 make cmake libtool autoconf automake \
    gcc gcc-c++ gcc-gfortran glibc-devel libgcc \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel zlib-devel libzip-devel \
    eigen3-devel armadillo-devel \
    expat-devel libcurl-devel openmpi-devel \
    flex-devel fftw3-devel \
    bzip2-devel \
    qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtcharts-devel \
    hdf5-devel netcdf-devel \
    gnuplot ImageMagick-devel ImageMagick-c++-devel \
    xorg-x11-xauth xorg-x11-apps \
    rpm-build redhat-rpm-config \
    rpm-devel rpmdevtools
  alternatives --set python /usr/bin/python3

  # create link for qtmake

  cd /usr/bin; \
  ln -s qmake-qt6 qmake;
```

## Manually install packages for RHEL 9, ALMA LINUX 9, ROCKY 9

```
  dnf install -y epel-release; \
  dnf install -y 'dnf-command(config-manager)'; \
  dnf install -y \
    tcsh wget git \
    emacs rsync python mlocate \
    python-devel platform-python-devel \
    m4 make cmake libtool autoconf automake \
    gcc gcc-c++ gcc-gfortran glibc-devel ; \
  dnf install -y --allowerasing \
    libX11-devel libXext-devel libcurl-devel \
    libpng-devel libtiff-devel zlib-devel libzip \
    expat-devel libcurl-devel openmpi-devel \
    flex fftw3-devel; \
  dnf install -y --allowerasing \
    bzip2-devel qt6-qtbase-devel qt6-qtdeclarative-devel \
    hdf5-devel netcdf-devel \
    xorg-x11-xauth \
    rpm-build redhat-rpm-config \
    rpm-devel rpmdevtools ; \
  cd /usr/bin; \
  ln -s qmake-qt6 qmake;
```

Also, if you are doing development, download tkdiff from:

```
* https://sourceforge.net/projects/tkdiff/
```

and install in

```
* ~/lrose/bin
```

## Manually install packages FEDORA

```
  yum install -y epel-release

  yum install -y \
    tcsh wget git \
    tkcvs emacs rsync python mlocate \
    m4 make cmake libtool autoconf automake \
    gcc gcc-c++ gcc-gfortran glibc-devel libgcc \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel zlib-devel libzip-devel \
    eigen3-devel armadillo-devel \
    expat-devel libcurl-devel openmpi-devel \
    flex-devel fftw3-devel \
    bzip2-devel qt6-qtbase-devel qt6-qtdeclarative-devel \
    hdf5-devel netcdf-devel \
    xorg-x11-xauth xorg-x11-apps \
    rpm-build redhat-rpm-config \
    rpm-devel rpmdevtools

  # create link for qtmake

  cd /usr/bin; \
  ln -s qmake-qt6 qmake;
```

## Manually install packages on Debian and Ubuntu

```
  apt-get update && \
  apt-get install -y \
    tcsh git gcc g++ gfortran rsync chrpath \
    automake make cmake mlocate libtool pkg-config python \
    libcurl3-dev curl \
    libfl-dev libbz2-dev libx11-dev libpng-dev \
    libfftw3-dev libexpat1-dev \
    qtbase5-dev qtdeclarative5-dev \
    libeigen3-dev libzip-dev \
    libarmadillo-dev libopenmpi-dev \
    libnetcdf-dev libhdf5-dev hdf5-tools \
    libcurl4-openssl-dev

  # create link for qmake

  cd /usr/bin; \
  /bin/rm -f qmake qmake-qt6; \
  ln -s /usr/lib/x86_64-linux-gnu/qt6/bin/qmake qmake; \
  ln -s /usr/lib/x86_64-linux-gnu/qt6/bin/qmake qmake-qt6
```

## Manually install packages on SUSE-based

Suse uses the same package lists as RedHat.

Use ```zypper``` instead of ```yum```, with the same package list as above.

```
  zypper install -y \
    tcsh wget git \
    tkdiff emacs rsync python docker \
    m4 make cmake libtool autoconf automake \
    gcc gcc-c++ gcc-fortran glibc-devel \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel zlib-devel \
    libexpat-devel libcurl-devel \
    flex fftw3-devel \
    libbz2-devel libzip-devel \
    libqt6-qtbase-devel libqt6-qtdeclarative-devel qt6-qtcharts-devel \
    eigen3-devel \
    hdf5-devel netcdf-devel \
    armadillo-devel openmpi-devel \
    xorg-x11-xauth \
    rpm-build rpm-devel rpmdevtools

  # create link for qtmake

  cd /usr/bin; \
  ln -s qmake-qt6 qmake;
```

## CIDD build - 32-bit packages required

See:

* [lrose_package_dependencies.cidd.md](./lrose_package_dependencies.cidd.md)

