#!/usr/bin/env python3

#===========================================================================
#
# Install linux package dependencies for LROSE
#
#===========================================================================

from __future__ import print_function
import os
import sys
import shutil
import subprocess
from optparse import OptionParser
import time
from datetime import datetime
from datetime import date
from datetime import timedelta
import glob
from sys import platform

def main():

    # globals

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global options
    global osType, osVersion

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    prefixDefault = '/usr/local/lrose'

    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--cidd32',
                      dest='cidd32', default=False,
                      action="store_true",
                      help='Install 32-bit dependencies for CIDD apps')
    parser.add_option('--osFile',
                      dest='osFile', default="/etc/os-release",
                      help='OS file path')

    (options, args) = parser.parse_args()
    if (options.verbose):
        options.debug = True
    
    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    dateStr = nowTime.strftime("%Y%m%d")
    timeStr = nowTime.strftime("%Y%m%d%H%M%S")
    dateTimeStr = nowTime.strftime("%Y/%m/%d-%H:%M:%S")

    # get the OS type and version
    
    getOsType()

    # let users know what we are doing

    print("****************************************************", file=sys.stderr)
    print("Running", thisScriptName, file=sys.stderr)
    print(" ", dateTimeStr, file=sys.stderr)
    print("  OS file: ", options.osFile, file=sys.stderr)
    print("  cidd32: ", options.cidd32, file=sys.stderr)
    print("  Installing dependencies for LROSE", file=sys.stderr)
    print("  OS type: ", osType, file=sys.stderr)
    print("  OS version: ", osVersion, file=sys.stderr)
    print("****************************************************", file=sys.stderr)

    # install the relevant packages

    if (osType == "centos"):
        if (osVersion == 6):
            installPackagesCentos6()
        elif (osVersion == 7):
            installPackagesCentos7()
        else:
            installPackagesCentos8()
    elif (osType == "almalinux"):
        installPackagesAlmalinux8()
    elif (osType == "fedora"):
         installPackagesFedora()
    elif (osType == "debian"):
         installPackagesDebian()
    elif (osType == "ubuntu"):
         installPackagesDebian()
    elif (osType == "suse"):
         installPackagesSuse()
    elif (osType == "oracle"):
         installPackagesOracle()
    else:
        print("ERROR - unsupported OS type: ", osType, " version: ", osVersion, file=sys.stderr)
            
    # done
    
    print("****************************************************", file=sys.stderr)
    print("Done", file=sys.stderr)
    print("****************************************************", file=sys.stderr)

    sys.exit(0)

########################################################################
# install packages for CENTOS 6

def installPackagesCentos6():

    # install epel

    shellCmd("yum install -y epel-release")

    # install main packages

    shellCmd("yum install -y tcsh wget git tkcvs " +
             "emacs rsync python " +
             "m4 make cmake libtool autoconf automake " +
             "gcc gcc-c++ gcc-gfortran glibc-devel " +
             "libX11-devel libXext-devel " +
             "libpng-devel libtiff-devel zlib-devel " +
             "expat-devel libcurl-devel " +
             "flex-devel fftw3-devel " +
             "bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel " +
             "hdf5-devel netcdf-devel " +
             "xorg-x11-xauth xorg-x11-apps " +
             "rpm-build redhat-rpm-config " +
             "rpm-devel rpmdevtools ")

    # install required 32-bit packages for CIDD

    if (options.cidd32):
        shellCmd("yum install -y " +
                 "xrdb Xvfb gnuplot " +
                 "glibc-devel.i686 libX11-devel.i686 libXext-devel.i686 " +
                 "libtiff-devel.i686 libpng-devel.i686 " +
                 "libstdc++-devel.i686 libgcc.i686 " +
                 "expat-devel.i686 flex-devel.i686 " +
                 "fftw-devel.i686 zlib-devel.i686 bzip2-devel.i686 " +
                 "xorg-x11-fonts-100dpi xorg-x11-fonts-ISO8859-1-100dpi " +
                 "xorg-x11-fonts-75dpi xorg-x11-fonts-ISO8859-1-75dpi " +
                 "xorg-x11-fonts-misc")

    # create link for qtmake

    shellCmd("cd /usr/bin; ln -f -s qmake-qt5 qmake")
    
    # install updated gcc and g++ toolchain

    shellCmd("wget http://people.centos.org/tru/devtools-2/devtools-2.repo -O /etc/yum.repos.d/devtools-2.repo")
    shellCmd("yum install -y devtoolset-2-gcc devtoolset-2-binutils")
    shellCmd("yum install -y devtoolset-2-gcc-c++ devtoolset-2-gcc-gfortran")

    # copy the updated compilers into /usr
    # so that they become the system default

    shellCmd("cd /opt/rh/devtoolset-2/root; rsync -av usr /")

########################################################################
# install packages for CENTOS 7

def installPackagesCentos7():

    # install epel

    shellCmd("yum install -y epel-release")

    # install main packages
    # break this up into pieces so it does not crash inside docker

    shellCmd("yum install -y " +
             "tcsh wget git " +
             "tkcvs emacs rsync python mlocate " +
             "m4 make cmake cmake3 libtool autoconf automake " +
             "gcc gcc-c++ gcc-gfortran glibc-devel " +
             "libX11-devel libXext-devel " +
             "libpng-devel libtiff-devel zlib-devel libzip-devel " +
             "eigen3-devel armadillo-devel ")
    shellCmd("yum install -y " +
             "expat-devel libcurl-devel openmpi-devel " +
             "flex-devel fftw3-devel " +
             "bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel " +
             "hdf5-devel netcdf-devel " +
             "xorg-x11-xauth xorg-x11-apps " +
             "rpm-build redhat-rpm-config " +
             "rpm-devel rpmdevtools")

    # install required 32-bit packages for CIDD
    
    if (options.cidd32):
        shellCmd("yum install -y " +
                 "xrdb Xvfb gnuplot " +
                 "glibc-devel.i686 libX11-devel.i686 libXext-devel.i686 " +
                 "libtiff-devel.i686 libpng-devel.i686 libcurl-devel.i686" +
                 "libstdc++-devel.i686 libgcc.i686 " +
                 "expat-devel.i686 flex-devel.i686 " +
                 "fftw-devel.i686 zlib-devel.i686 bzip2-devel.i686 " +
                 "xorg-x11-fonts-100dpi xorg-x11-fonts-ISO8859-1-100dpi " +
                 "xorg-x11-fonts-75dpi xorg-x11-fonts-ISO8859-1-75dpi " +
                 "xorg-x11-fonts-misc")

    # create link for qtmake

    shellCmd("cd /usr/bin; ln -f -s qmake-qt5 qmake")
    
########################################################################
# install packages for CENTOS 8

def installPackagesCentos8():

    # install epel

    shellCmd("dnf install -y epel-release python2 python3")
    shellCmd("dnf install -y 'dnf-command(config-manager)'")
    shellCmd("dnf config-manager --set-enabled powertools")
    shellCmd("alternatives --set python /usr/bin/python3")

    # install main packages
    # break this up into pieces so it does not crash inside docker

    shellCmd("dnf install -y --allowerasing " +
             "tcsh wget git " +
             "emacs rsync python2 python3 mlocate " +
             "python2-devel platform-python-devel " +
             "m4 make cmake libtool autoconf automake " +
             "gcc gcc-c++ gcc-gfortran glibc-devel")

    shellCmd("dnf install -y --allowerasing " +
             "libX11-devel libXext-devel libcurl-devel " +
             "libpng-devel libtiff-devel zlib-devel libzip-devel " +
             "eigen3-devel armadillo-devel " +
             "expat-devel libcurl-devel openmpi-devel " +
             "flex-devel fftw3-devel ")

    shellCmd("dnf install -y --allowerasing " +
             "bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel " +
             "hdf5-devel netcdf-devel " +
             "xorg-x11-xauth xorg-x11-apps " +
             "rpm-build redhat-rpm-config " +
             "rpm-devel rpmdevtools")

    # install required 32-bit packages for CIDD
    
    if (options.cidd32):
        shellCmd("dnf install -y --allowerasing " +
                 "xrdb " +
                 "glibc-devel.i686 libX11-devel.i686 libXext-devel.i686 " +
                 "libcurl-devel.i686 " +
                 "libtiff-devel.i686 libpng-devel.i686 " +
                 "libstdc++-devel.i686 libtiff-devel.i686 ")
        shellCmd("dnf install -y --allowerasing " +
                 "zlib-devel.i686 expat-devel.i686 flex-devel.i686 " +
                 "fftw-devel.i686 bzip2-devel.i686 " +
                 "gnuplot ImageMagick-devel ImageMagick-c++-devel " +
                 "xorg-x11-fonts-100dpi xorg-x11-fonts-ISO8859-1-100dpi " +
                 "xorg-x11-fonts-75dpi xorg-x11-fonts-ISO8859-1-75dpi " +
                 "xorg-x11-fonts-misc")

    # create link for qtmake

    shellCmd("cd /usr/bin; ln -f -s qmake-qt5 qmake")
    
########################################################################
# install packages for ALMALINUX 8

def installPackagesAlmalinux8():

    # install epel

    shellCmd("dnf install -y epel-release python2 python3")
    shellCmd("dnf install -y 'dnf-command(config-manager)'")
    shellCmd("dnf config-manager --set-enabled powertools")
    shellCmd("alternatives --set python /usr/bin/python3")

    # install main packages
    # break this up into pieces so it does not crash inside docker

    shellCmd("dnf install -y --allowerasing " +
             "tcsh wget git " +
             "emacs rsync python2 python3 mlocate " +
             "python2-devel platform-python-devel " +
             "m4 make cmake libtool autoconf automake " +
             "gcc gcc-c++ gcc-gfortran glibc-devel")

    shellCmd("dnf install -y --allowerasing " +
             "libX11-devel libXext-devel libcurl-devel " +
             "libpng-devel libtiff-devel zlib-devel libzip-devel " +
             "eigen3-devel armadillo-devel " +
             "expat-devel libcurl-devel openmpi-devel " +
             "flex-devel fftw3-devel ")

    shellCmd("dnf install -y --allowerasing " +
             "bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel " +
             "hdf5-devel netcdf-devel " +
             "xorg-x11-xauth xorg-x11-apps " +
             "rpm-build redhat-rpm-config " +
             "rpm-devel rpmdevtools")

    # install required 32-bit packages for CIDD
    
    if (options.cidd32):
        shellCmd("dnf install -y --allowerasing " +
                 "xrdb " +
                 "glibc-devel.i686 libX11-devel.i686 libXext-devel.i686 " +
                 "libcurl-devel.i686 " +
                 "libtiff-devel.i686 libpng-devel.i686 " +
                 "libstdc++-devel.i686 libtiff-devel.i686 ")
        shellCmd("dnf install -y --allowerasing " +
                 "zlib-devel.i686 expat-devel.i686 flex-devel.i686 " +
                 "fftw-devel.i686 bzip2-devel.i686 " +
                 "gnuplot ImageMagick-devel ImageMagick-c++-devel " +
                 "xorg-x11-fonts-100dpi xorg-x11-fonts-ISO8859-1-100dpi " +
                 "xorg-x11-fonts-75dpi xorg-x11-fonts-ISO8859-1-75dpi " +
                 "xorg-x11-fonts-misc")

    # create link for qtmake

    shellCmd("cd /usr/bin; ln -f -s qmake-qt5 qmake")
    
########################################################################
# install packages for FEDORA

def installPackagesFedora():

    # install main packages
    # break this up into pieces so it does not crash inside docker

    shellCmd("yum install -y " +
             "tcsh wget git " +
             "tkcvs emacs rsync python mlocate " +
             "m4 make cmake libtool autoconf automake " +
             "gcc gcc-c++ gcc-gfortran glibc-devel " +
             "libX11-devel libXext-devel " +
             "libpng-devel libtiff-devel zlib-devel libzip-devel " +
             "eigen3-devel armadillo-devel ")
    shellCmd("yum install -y " +
             "expat-devel libcurl-devel openmpi-devel " +
             "flex-devel fftw3-devel " +
             "bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel " +
             "hdf5-devel netcdf-devel " +
             "xorg-x11-xauth xorg-x11-apps " +
             "rpm-build redhat-rpm-config " +
             "rpm-devel rpmdevtools")

    # install required 32-bit packages for CIDD
    
    if (options.cidd32):
        shellCmd("yum install -y " +
                 "xrdb Xvfb gnuplot " +
                 "glibc-devel.i686 libX11-devel.i686 libXext-devel.i686 " +
                 "libtiff-devel.i686 libpng-devel.i686 libcurl-devel.i686 " +
                 "libstdc++-devel.i686 libgcc.i686 " +
                 "expat-devel.i686 flex-devel.i686 " +
                 "fftw-devel.i686 zlib-devel.i686 bzip2-devel.i686 " +
                 "ImageMagick-devel ImageMagick-c++-devel " +
                 "xorg-x11-fonts-100dpi xorg-x11-fonts-ISO8859-1-100dpi " +
                 "xorg-x11-fonts-75dpi xorg-x11-fonts-ISO8859-1-75dpi " +
                 "xorg-x11-fonts-misc")

    # create link for qtmake

    shellCmd("cd /usr/bin; ln -f -s qmake-qt5 qmake")

########################################################################
# install packages for ORACLE 8

def installPackagesOracle():

    # install epel

    shellCmd("dnf install -y oracle-epel-release-el8 python2 python3")
    shellCmd("dnf install -y 'dnf-command(config-manager)'")
    shellCmd("alternatives --set python /usr/bin/python3")

    # install main packages
    # break this up into pieces so it does not crash inside docker

    shellCmd("dnf install -y --allowerasing " +
             "tcsh wget git " +
             "emacs rsync python2 python3 mlocate " +
             "python2-devel platform-python-devel " +
             "m4 make cmake libtool autoconf automake " +
             "gcc gcc-c++ gcc-gfortran glibc-devel")

    shellCmd("dnf install -y --allowerasing " +
             "libX11-devel libXext-devel libcurl-devel " +
             "libpng-devel libtiff-devel zlib-devel libzip-devel " +
             "expat-devel libcurl-devel openmpi-devel " +
             "flex fftw3-devel ")

    shellCmd("dnf install -y --allowerasing " +
             "bzip2-devel qt5-qtbase-devel qt5-qtdeclarative-devel " +
             "xorg-x11-xauth " +
             "rpm-build redhat-rpm-config " +
             "rpm-devel rpmdevtools")

    # create link for qtmake

    shellCmd("cd /usr/bin; ln -f -s qmake-qt5 qmake")
    
########################################################################
# install packages for Debian

def installPackagesDebian():

    # set the environment

    os.environ["DEBIAN_FRONTEND"] = "noninteractive"

    # install main packages
    
    shellCmd("apt-get -y update")
    shellCmd("apt-get install -y " +
             "tcsh git gcc g++ gfortran rsync chrpath " +
             "automake make cmake mlocate libtool pkg-config python " +
             "libcurl3-dev curl " +
             "libfl-dev libbz2-dev libx11-dev libpng-dev " +
             "libfftw3-dev libexpat1-dev " +
             "qtbase5-dev qtdeclarative5-dev " +
             "libeigen3-dev libzip-dev " +
             "libarmadillo-dev libopenmpi-dev " +
             "libnetcdf-dev libhdf5-dev hdf5-tools " +
             "libcurl4-openssl-dev")

    # create link for qmake

    shellCmd("cd /usr/bin; " +
             "/bin/rm -f qmake qmake-qt5; " +
             "ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake; " +
             "ln -s /usr/lib/x86_64-linux-gnu/qt5/bin/qmake qmake-qt5")

    # install packages for running CIDD

    if (options.cidd32):
        shellCmd("/usr/bin/dpkg --add-architecture i386")
        shellCmd("apt-get -y update")
        shellCmd("apt-get install -y " +
                 "libx11-dev:i386 " +
                 "libxext-dev:i386 " +
                 "libfftw3-dev:i386 " +
                 "libexpat-dev:i386 " +
                 "libpng-dev:i386 " +
                 "libfl-dev:i386 " +
                 "libbz2-dev:i386 " +
                 "libzip-dev:i386")

########################################################################
# install packages for suse

def installPackagesSuse():

    # install main packages

    shellCmd("zypper install -y tcsh wget git " +
             "tkdiff emacs rsync python docker " +
             "m4 make cmake libtool autoconf automake " +
             "gcc gcc-c++ gcc-fortran glibc-devel " +
             "libX11-devel libXext-devel " +
             "libpng-devel libtiff-devel zlib-devel " +
             "libexpat-devel libcurl-devel " +
             "flex fftw3-devel ")
    shellCmd("zypper install -y tcsh wget git " +
             "libbz2-devel libzip-devel " +
             "libqt5-qtbase-devel libqt5-qtdeclarative-devel " +
             "eigen3-devel " +
             "hdf5-devel netcdf-devel " +
             "armadillo-devel openmpi-devel " +
             "xorg-x11-xauth " +
             "rpm-build rpm-devel rpmdevtools")

    # install packages for CIDD

    if (options.cidd32):
        shellCmd("zypper install -y " +
                 "xrdb " +
                 "glibc-devel-32bit libX11-devel-32bit libXext-devel-32bit " +
                 "libtiff-devel-32bit libpng-devel-32bit libcurl-devel-32bit " +
                 "libstdc++-devel-32bit libtiff-devel-32bit " +
                 "zlib-devel-32bit libexpat-devel-32bit flex-32bit " +
                 "libfftw3-3-32bit libbz2-devel-32bit " +
                 "gnuplot ImageMagick-devel-32bit " +
                 "xorg-x11 xorg-x11-devel xorg-x11-fonts xorg-x11-fonts-core")

    # create link for qtmake

    shellCmd("cd /usr/bin; ln -f -s qmake-qt5 qmake")
    
########################################################################
# get the OS type from the /etc/os-release file in linux

def getOsType():

    global osType, osVersion

    # Mac OSX?

    if (platform == "darwin"):
        osType = "OSX"
        osVersion = 10
        return

    # not linux?

    if (platform.find("linux") < 0):
        osType = "unknown"
        osVersion = 0
        return

    # Centos 6?

    if (os.path.isfile("/etc/redhat-release")):
        rhrelease_file = open("/etc/redhat-release", "rt")
        lines = rhrelease_file.readlines()
        rhrelease_file.close()
        for line in lines:
          if (line.find("CentOS release 6") == 0):
            osType = "centos"
            osVersion = 6
            return

    # get os info from /etc/os-release file
  
    _file = open(options.osFile, "rt")
    lines = _file.readlines()
    _file.close()

    osType = "unknown"
    for line in lines:

        line.strip()
        
        if (line.find("NAME") == 0):
          nameline = line.lower()
          if (nameline.find("centos") >= 0):
            osType = "centos"
          elif (nameline.find("almalinux") >= 0):
            osType = "almalinux"
          elif (nameline.find("fedora") >= 0):
            osType = "fedora"
          elif (nameline.find("debian") >= 0):
            osType = "debian"
          elif (nameline.find("ubuntu") >= 0):
            osType = "ubuntu"
          elif (nameline.find("suse") >= 0):
            osType = "suse"
          elif (nameline.find("oracle") >= 0):
            osType = "oracle"
            
        if (line.find("VERSION_ID") == 0):
          lineParts = line.split('=')
          print("  lineParts: ", lineParts, file=sys.stderr)
          if (lineParts[1].find('"') >= 0):
            subParts = lineParts[1].split('"')
            print("  subParts: ", subParts, file=sys.stderr)
            osVersion = float(subParts[1])
          else:
            osVersion = float(lineParts[1])

          return

########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    if (options.debug):
        print("running cmd:", cmd, " .....", file=sys.stderr)
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            sys.exit(1)
        else:
            if (options.verbose):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, file=sys.stderr)
        sys.exit(1)

    if (options.debug):
        print(".... done", file=sys.stderr)
    
########################################################################
# Run - entry point

if __name__ == "__main__":
    main()
