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

FROM i686/ubuntu

# install required packages

RUN apt-get -y update
RUN apt-get -y install libx11-dev fftw3-dev libpng-dev libexpat-dev libxext-dev make g++ git python libz-dev libbz2-dev automake autoconf x11-apps xauth 

# get and build hdf5/netcdf

RUN mkdir /tmp/cidd
RUN cd /tmp/cidd; git clone https://github.com/NCAR/lrose-netcdf
RUN cd /tmp/cidd/lrose-netcdf; ./build_and_install_netcdf.m32 -x /tmp/cidd

# get lrose-core
    
RUN cd /tmp/cidd; git clone https://github.com/NCAR/lrose-core

# set up build environment

ENV HOST_OS CIDD_32
ENV RAP_MAKE_INC_DIR /tmp/cidd/lrose-core/codebase/make_include
ENV RAP_MAKE_BIN_DIR /tmp/cidd/lrose-core/codebase/make_bin
ENV RAP_INC_DIR /tmp/cidd/include
ENV RAP_LIB_DIR /tmp/cidd/lib
ENV RAP_BIN_DIR /tmp/cidd/bin
ENV RAP_SHARED_INC_DIR /tmp/cidd/include
ENV RAP_SHARED_LIB_DIR /tmp/cidd/lib
ENV RAP_SHARED_BIN_DIR /tmp/cidd/bin

# install cidd makefiles

RUN cd /tmp/cidd/lrose-core/codebase; ls -al
RUN cd /tmp/cidd/lrose-core; ./codebase/make_bin/install_package_makefiles.py --package cidd --debug

# build libraries

RUN cd /tmp/cidd/lrose-core/codebase/libs; make -j 8 install_include
RUN cd /tmp/cidd/lrose-core/codebase/libs; make -j 8 opt
RUN cd /tmp/cidd/lrose-core/codebase/libs; make -j 8 install

# build apps

RUN cd /tmp/cidd/lrose-core/codebase/apps; make -j 8 opt
RUN cd /tmp/cidd/lrose-core/codebase/apps; make -j 8 install

#ADD . /usr/local/src


# ENTRYPOINT [ "/usr/sbin/httpd" ]
CMD [ "/tmp/cidd/bin/CIDD", "-v 1" ]
#CMD [ "/tmp/cidd/bin/CIDD", "-p http://front.eol.ucar.edu/displayParams/CIDD.relampago" ]
