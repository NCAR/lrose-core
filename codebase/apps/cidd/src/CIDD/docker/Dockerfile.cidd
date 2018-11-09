# FROM registry.access.redhat.com/rhel7/rhel
# RUN yum -y install httpd && yum -y update; yum clean all

# To run ...
#
# make sure XQuartz is installed and accepts connections from local hosts
#
# add your current ifconfig net0 address to xhosts
#
# xhost +xxx.xx.xx.x
# docker run -ti --rm -e DISPLAY=xxx.xx.xx.x:0 -v /tmp/.X11-unix:/tmp/.X11-unix:rw i686ubuntu_prep

# get the 32-bit image as a starting pt

FROM i386/centos

# install required packages

RUN \
  yum -y update; \
  yum install -y tcsh perl perl-Env python \
  git emacs \
  make automake autoconf \
  gcc gcc-c++ gcc-gfortran \
  libtiff-devel libpng-devel \
  libstdc++-devel libtiff-devel \
  zlib-devel expat-devel flex-devel \
  fftw-devel bzip2-devel xrdb Xvfb \
  gnuplot ImageMagick-devel ImageMagick-c++-devel \
  glibc-devel libX11-devel libXext-devel \
  xorg-x11-xauth xorg-x11-apps \
  xorg-x11-fonts-100dpi xorg-x11-fonts-ISO8859-1-100dpi \
  xorg-x11-fonts-75dpi xorg-x11-fonts-ISO8859-1-75dpi \
  xorg-x11-fonts-misc

# get and build hdf5/netcdf

RUN mkdir -p /usr/local; mkdir -p /usr/local/src
RUN cd /usr/local/src; git clone https://github.com/NCAR/lrose-netcdf
RUN cd /usr/local/src/lrose-netcdf; ./build_and_install_netcdf.m32 -x /usr/local

# get lrose-core
    
RUN cd /usr/local/src; git clone https://github.com/NCAR/lrose-core

# set up build environment

ENV HOST_OS CIDD_32
ENV RAP_MAKE_INC_DIR /usr/local/src/lrose-core/codebase/make_include
ENV RAP_MAKE_BIN_DIR /usr/local/src/lrose-core/codebase/make_bin
ENV RAP_INC_DIR /usr/local/include
ENV RAP_LIB_DIR /usr/local/lib
ENV RAP_BIN_DIR /usr/local/bin
ENV RAP_SHARED_INC_DIR /usr/local/include
ENV RAP_SHARED_LIB_DIR /usr/local/lib
ENV RAP_SHARED_BIN_DIR /usr/local/bin

# install cidd makefiles

RUN \
  cd /usr/local/src/lrose-core; \
  ./codebase/make_bin/install_package_makefiles.py --package cidd --debug

# build libraries

RUN \
  cd /usr/local/src/lrose-core/codebase/libs; make -j 8 install_include; \
  cd /usr/local/src/lrose-core/codebase/libs; make -j 8 opt; \
  cd /usr/local/src/lrose-core/codebase/libs; make -j 8 install

# build apps

RUN \
  cd /usr/local/src/lrose-core/codebase/apps; make -j 8 opt; \ 
  cd /usr/local/src/lrose-core/codebase/apps; make -j 8 install

# checkout the relampago project

RUN mkdir -p /usr/local/git
RUN cd /usr/local/git; git clone https://github.com/NCAR/lrose-projects-relampago

#
# this is critical to X11 forwarding
#
#CMD mkdir /etc; dbus-uuidgen > /etc/machine-id 

# set the entrypoint

#RUN cp /usr/local/src/lrose-core/codebase/apps/cidd/src/CIDD/docker/start_CIDD.relampago /usr/local/bin/cidd-entrypoint

COPY entrypoint.sh /usr/local/bin/cidd-entrypoint
ENTRYPOINT ["/usr/local/bin/cidd-entrypoint"]
