FROM centos
# FROM centos:7

ADD . /tmp/bj
WORKDIR /tmp/bj

#
# is there a way to pull the source release from github?
#
RUN yum -y install rsync
RUN yum -y install gcc 
RUN yum -y install gcc-gfortran
RUN yum -y install gcc-c++
RUN yum -y install make
#RUN yum -y install wget
RUN yum -y install expat-devel
RUN yum -y install m4
RUN yum -y install jasper-devel
RUN yum -y install flex-devel
RUN yum -y install zlib-devel
RUN yum -y install libpng-devel
RUN yum -y install bzip2-devel
RUN yum -y install qt5-qtbase-devel
RUN yum -y install fftw3-devel
RUN yum install -y xorg-x11-server-Xorg xorg-x11-xauth xorg-x11-apps

#RUN yum -y install libX11-devel

#RUN  cd /tmp/bj
#
RUN tar xvfz lrose-blaze-20180516.src.tar.gz # ???
RUN cd lrose-blaze-20180516.src; ./build_src_release.py

# create a user
# RUN useradd -ms /bin/bash lrose 
# USER lrose
# WORKDIR /home/lrose

#
# this is critical to X11 forwarding
#
CMD dbus-uuidgen > /etc/machine-id 

# use this to test X11 forwarding
#
# CMD xclock&
