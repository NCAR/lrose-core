###############################################################
# start with clean image
# provision with the required packages

FROM centos:6

# install required packages

RUN \
    yum install -y epel-release; \
    yum install -y tcsh wget git tkcvs \
    emacs rsync python \
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

# install updated gcc and g++ toolchain

RUN \
    wget http://people.centos.org/tru/devtools-2/devtools-2.repo -O /etc/yum.repos.d/devtools-2.repo; \
    yum install -y devtoolset-2-gcc devtoolset-2-binutils; \
    yum install -y devtoolset-2-gcc-c++ devtoolset-2-gcc-gfortran

# copy the updated compilers into /usr
# so that they become the system default

RUN \
    cd /opt/rh/devtoolset-2/root; \
    rsync -av usr /

# Set the path to the new compilers
#    cd; \
#    cat .bashrc /opt/rh/devtoolset-2/enable > /tmp/bashrc; \
#    /bin/mv -f /tmp/bashrc .bashrc



