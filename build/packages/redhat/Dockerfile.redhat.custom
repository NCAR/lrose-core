#==============================================================
# provision a Docker image for building lrose
# start with clean image
# install the required packages

ARG OS_TYPE=centos

# install required packages

RUN \
    echo "OS_TYPE: ${OS_TYPE}"; \
    if [ "${OS_TYPE}" = centos ]; then \
       echo "==>> install epel-release <<=="; \
       yum install -y epel-release; \
    fi

RUN \
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
    rpm-devel rpmdevtools

# create link for qtmake

RUN \
    cd /usr/bin; \
    ln -s qmake-qt5 qmake;

# 32-bit libs etc for CIDD build

RUN \
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
