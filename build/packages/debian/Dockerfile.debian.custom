#===================================================
# create a custom Docker image for building lrose
# start with clean image
# install the required packages
#
# This file will be pre-pended with the FROM command

# install required packages

RUN apt-get update && \
    apt-get install -y \
    git gcc g++ gfortran cmake rsync mlocate \
    automake make libtool pkg-config python \
    libcurl3-dev curl \
    libfl-dev libbz2-dev libx11-dev libpng-dev \
    qtbase5-dev qtdeclarative5-dev \
    libfftw3-dev libexpat1-dev \
    libgeographic-dev libeigen3-dev libzip-dev \
    libnetcdf-dev netcdf-bin libhdf5-dev hdf5-tools

# create link for qtmake

RUN \
    cd /usr/bin; \
    /bin/rm -f qmake qmake-qt5; \
    ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake; \
    ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake-qt5


