# Download LROSE binary release and install - LINUX

1. [prepare](#prepare)
2. [download](#download)
3. [install](#install)
4. [verify](#verify)

<a name="prepare"/>

## 1. Prepare

The current LINUX binary release is built under CENTOS 7.5.
This should run on most up-to-date RedHat-based systems.

Recommended distributions are:

  * RedHat
  * Centos (based on RedHat)
  * Fedora (based on RedHat)

For the Qt apps (e.g. HawkEye) to run, make sure Qt6 is installed:

```
  yum install -y qt6-qtbase-devel qt6-qtdeclarative-devel
```

<a name="download"/>

## 2. Download

Choose your install directory (prefix location). The default is:

```
  ~/lrose
```

Download the binary tar file from:

```
  https://github.com/NCAR/lrose-core/releases
```

A typical binary release would be:

```
  lrose-20181223.bin.x84_64.tgz
```

### Untar the release

```
  cd ~/Downloads
  tar xvfz lrose-20181223.bin.x86_64.tgz
```

The release will be unpacked into a subdirectory:

```
  ~/Downloads/lrose-20181223.bin.x86_64
```

So go there:

```
  cd ~/Downloads/lrose-20181223.bin.x86_64
```

<a name="install"/>

## 3. Install

### Run the build script:

To see the install script usage:

```
  ./install_bin_release.py --help
```

To install into the default directory `~/lrose` run:

```
  ./install_bin_release.py
```

The dynamic run-time libraries will be found in:

```
  ~/lrose/bin/lrose_runtime_libs
```

i.e. in a subdirectory of the bin directory.

Or to specify the install directory, run something like the following:

```
  ./install_bin_release.py --prefix /my/install/dir
```

<a name="verify"/>

## 4. Verify

Look in ~/lrose or /my/install/dir for

```
  include
  lib
  bin
```

Try the commands:
```
  ~/lrose/bin/RadxPrint -h
  ~/lrose/bin/RadxConvert -h
  ~/lrose/bin/Radx2Grid -h
  ~/lrose/bin/HawkEye
```
