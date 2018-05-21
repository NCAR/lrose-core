## Installing an LROSE Source Release (Linux)

1. [prepare](#prepare)
2. [install](#install)
3. [verify](#verify)

<a name="prepare"/>

### 1. Prepare

LROSE depends on several packages.  Make sure you have these packages installed or install them using (yum or apt-get).
```
yum -y install rsync
yum -y install gcc 
yum -y install gcc-gfortran
yum -y install gcc-c++
yum -y install make
yum -y install expat-devel
yum -y install m4
yum -y install jasper-devel
yum -y install flex-devel
yum -y install zlib-devel
yum -y install libpng-devel
yum -y install bzip2-devel
yum -y install fftw3-devel
yum -y install qt5-qtbase-devel
yum install -y xorg-x11-server-Xorg xorg-x11-xauth xorg-x11-apps
```

<a name="install"/>

### 2. Install

Create a working directory for the distribution:

```
  mkdir lrose_build
  cd lrose_build
```

#### Download the source release for Linux

Download the source tar file from:

```
  https://github.com/NCAR/lrose-core/releases 
```

A typical source release would be:

```
  lrose-blaze-20160823.src.tgz
```

#### Untar

```
  tar xvfz lrose-blaze-20160823.src.tgz
```

The distribution will be unpacked into a subdirectory:

```
  lrose_build/lrose-blaze-20160823.src
```

#### Run the build script:

Choose an install directory. The default is: `/usr/local/lrose`

```
  ./build_src_release.py
```
or

```
  ./build_src_release.py --prefix /my/install/dir
```

<a name="verify"/>

### 3. Verify

Look in /usr/local/lrose or /my/install/dir for

```
  include
  lib
  bin
```

Try the commands:
```
/usr/local/lrose/bin/RadxPrint -h
/usr/local/lrose/bin/RadxConvert -h
/usr/local/lrose/bin/Radx2Grid -h
/usr/local/lrose/bin/HawkEye
```

### Handling build errors

If the build does not complete successfully, you will need to
track down the errors. It is the first errors in the build that
are the most important.

If you get errors, go into the directory giving problems, and
run the make as follows:

```
  make |& less
```

and scroll for errors.

Alternatively, run

```
  make >& make.log
```

and then inspect the make.log file.

