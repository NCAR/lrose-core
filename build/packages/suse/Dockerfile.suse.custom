#==============================================================
# provision a Docker image for building lrose
# start with clean image
# install the required packages

ARG OS_TYPE=suse

# install required packages

RUN \
    zypper install -y tcsh wget git \
    tkdiff emacs rsync python docker \
    m4 make libtool autoconf automake \
    gcc gcc-c++ gcc-fortran glibc-devel \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel zlib-devel \
    libexpat-devel libcurl-devel \
    flex fftw3-devel \
    libbz2-devel libzip-devel \
    libqt5-qtbase-devel libqt5-qtdeclarative-devel \
    hdf5-devel netcdf-devel \
    xorg-x11-xauth \
    rpm-build rpm-devel rpmdevtools

# create link for qtmake

RUN \
    cd /usr/bin; \
    ln -s qmake-qt5 qmake;

# 32-bit libs etc for CIDD build

RUN \
    zypper install -y \
    xrdb \
    glibc-devel-32bit libX11-devel-32bit libXext-devel-32bit \
    libtiff-devel-32bit libpng-devel-32bit libcurl-devel-32bit \
    libstdc++-devel-32bit libtiff-devel-32bit \
    zlib-devel-32bit libexpat-devel-32bit flex-32bit \
    libfftw3-3-32bit libbz2-devel-32bit \
    gnuplot ImageMagick-devel-32bit \
    xorg-x11 xorg-x11-devel xorg-x11-fonts xorg-x11-fonts-core

