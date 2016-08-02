############################################################
#
# LINUX setup for LROSE
#
# This readme details how you should set up your LINUX
# system to prepare it for LROSE.
#
############################################################

1. Hardware requirements
------------------------

CPU: 3.0+ GHz 32-bit, or 2.5+ GHz 64-bit.
     Dual- or quad-core are good.

RAM: 4+ GBytes

Disk: 500 GB+

Graphics: 256 MByte, 1600 x 1200 or better

2. LINUX distribution
---------------------

Most good, up-to date distributions should work.

Recommended distributions are:

  Debian
  Ubuntu
  RedHat
  Centos
  ScientificLinux
  Fedora

3. Packages
-----------

Required development packages for compiling:

  tcsh
  perl
  perl-Env

  ftp
  git
  svn
  cvs
  tkcvs
  emacs

  gcc
  g++
  gfortran

  glibc-devel
  libX11-devel
  libXext-devel (if available)
  libpng-devel
  libtiff-devel
  jasper-devel
  zlib-devel
  libexpat-devel
  flex-devel
  fftw3-devel
  bzip2-devel
  jasper-devel
  qt4-devel

  gnuplot
  ImageMagick-devel
  ImageMagick-c++-devel

  xrdb
  Xvfb (virtual X server), specifically xorg-x11-server-Xvfb
  sshd (ssh logins)

  xorg-x11-fonts-misc
  xorg-x11-fonts-75dpi
  xorg-x11-fonts-100dpi

On Redhat-based hosts you can achieve this by running:

  yum install -y tcsh perl perl-Env ftp git svn cvs tkcvs emacs gcc gcc-c++ gcc-gfortran glibc-devel libX11-devel libXext-devel libpng-devel libtiff-devel jasper-devel zlib-devel expat-devel flex-devel fftw3-devel bzip2-devel jasper-devel qt4-devel xrdb Xvfb xorg-x11-fonts-misc xorg-x11-fonts-75dpi xorg-x11-fonts-100dpi gnuplot ImageMagick-devel ImageMagick-c++-devel

If you need 32-bit compatibility (for CIDD), install the following packages:

  glibc-devel.i686
  libX11-devel.i686
  libXext-devel.i686
  libjpeg-devel.i686
  libpng-devel.i686
  libstdc++-devel.i686
  libtiff-devel.i686

On Redhat-based hosts you can achieve this by running:

  yum install -y glibc-devel.i686 libX11-devel.i686 libXext-devel.i686 libjpeg-devel.i686 libpng-devel.i686 libstdc++-devel.i686 libtiff-devel.i686

On Debian, you need to run the following:

  /usr/bin/dpkg --add-architecture i386

and use apt-get to install the following:

  libstdc++5:i386
  libstdc++6:i386
  libxml2:i386
  libgtk2.0-0:i386
  libgdk-pixbuf2.0-0:i386

4. Installing NetCDF
--------------------

This is done automatically as part of the LROSE build.

5. Opening up firewall
----------------------

To allow for socketing between hosts, you need to disable
the firewall.

On Centos 7:

  systemctl stop firewalld
  systemctl stop iptables

