#==============================================================
# Build CIDD in a CENTOS Docker image
#
# CIDD will be run from this image
#
# 1. install the required packages
# 2. perform the build

FROM centos

RUN \
# need to install epel-release first
    yum install -y epel-release; \
    \
# then install 64-bit packages
    yum install -y tcsh wget git \
    tkcvs emacs rsync python \
    m4 make libtool autoconf automake \
    gcc gcc-c++ gcc-gfortran glibc-devel \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel zlib-devel \
    expat-devel libcurl-devel \
    flex-devel fftw3-devel \
    bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel \
    hdf5-devel netcdf-devel \
    xorg-x11-xauth xorg-x11-apps \
    rpm-build redhat-rpm-config \
    rpm-devel rpmdevtools; \
    \
# then install 32-bit packages
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
    xorg-x11-fonts-misc; \
    \
#   then create link for qtmake
    cd /usr/bin; \
    ln -s qmake-qt5 qmake; \
# do the checkout and build
    /scripts/checkout_and_build_auto.py \
    --package lrose-cidd \
    --prefix /usr/local/cidd \
    --buildDir /tmp/cidd_build \
    --logDir /tmp/cidd_build_logs; \
# then clean up
    /bin/rm -rf /tmp/cidd_build /tmp/cidd_build_logs
