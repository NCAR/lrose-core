# Building CSU FRACTL

## checkout fractl

```
  git clone https://github.com/mmbell/fractl.git 
```

## install required packages

### On Redhat-based hosts, run the following (as root or sudo):

```
sudo yum install -y epel-release

sudo yum install -y \
    tcsh wget git \
    tkcvs emacs rsync python \
    m4 make cmake libtool autoconf automake \
    gcc gcc-c++ gcc-gfortran glibc-devel \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel \
    zlib-devel libzip-devel bzip2-devel \
    expat-devel libcurl-devel \
    flex-devel fftw3-devel \
    GeographicLib-devel eigen3-devel \
    bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel \
    hdf5-devel netcdf-devel \
    xorg-x11-xauth xorg-x11-apps \
    rpm-build redhat-rpm-config \
    rpm-devel rpmdevtools

cd /usr/bin
sudo ln -s qmake-qt5 qmake
```

### On Debian-based hosts run the following (as root or sudo):

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
    libnetcdf-dev netcdf-bin libhdf5-dev hdf5-tools \
    libgeographic-dev libeigen3-dev \
    libzip-dev libcurl4-openssl-dev

## do the build - uses cmake

```
  cd fractl
  cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local/lrose .
  make install
```

Note: this assumes lrose is installed in:

```
 /usr/local/lrose.
```

If it in a different location, change ```CMAKE_INSTALL_PREFIX```.

For example:

```
  cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/local/lrose .
  make install
```


