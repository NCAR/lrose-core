# Downloading and installing the CIDD binary release

The CIDD display is tricky to compile, requiring a 32-bit mode for the build.
We compile CIDD and package it up into a binary tar file, from which it can be installed.

## Download the tar file

You can find the tar file under the latest releases page on github:

* [lrose-core releases](https://github.com/NCAR/lrose-core/releases)

Here is an example name from the 2020/06/09 release:

```
  lrose-cidd-20200609.bin.x86_64.tgz
```

## Download the tar file

Untar the file into a temporary directory. Here we use the above file as an example:

```
  cd /tmp
  tar xvfz ~/Downloads/lrose-cidd-20200609.bin.x86_64.tgz
```

## Perform the installation

```
  cd /tmp/lrose-cidd-20200609.bin.x86_64
  ./install_cidd_bin_release.py
```

This will install the CIDD binaries into ```/usr/local/lrose/bin```.

The relevant files are:

```
  /usr/local/lrose/bin/CIDD
  /usr/local/lrose/bin/lrose-cidd_runtime_libs
```

The ```lrose-core_runtime_libs``` directory contains most of the 32-bit libraries that were used during the compile and build.

## Install 32-bit runtime support

To ensure that you have 32-bit runtime support for CIDD, you will need to install the relevant 32-bit packages.

The best way to do that is to run the `install_linux_packages.py` script from the `lrose-core` repository.

```
  wget https://raw.githubusercontent.com/NCAR/lrose-core/master/build/scripts/install_linux_packages.py
  chmod +x install_linux_packages.py
  ./install_linux_packages.py --cidd32
  rm install_linux_packages.py
```

The script will determine the operating system version from the `/etc/os-release` file and install the required packages for that version.

## Running CIDD

You should now be able to run CIDD.

The following is a good test:

```
  exec /bin/csh
  # set resources
  set timestr = `date -u +%Y%m%d%H%M%S`
  set XResourcesFile = /tmp/XResources4CIDD.${timestr}
  touch $XResourcesFile
  echo "OpenWindows.MonospaceFont: 7x13" >> $XResourcesFile
  echo "OpenWindows.RegularFont: 6x13" >> $XResourcesFile
  echo "OpenWindows.BoldFont: 6x13bold" >> $XResourcesFile
  xrdb -nocpp -override $XResourcesFile
  /bin/rm -f $XResourcesFile
  if (-e ~/.Xdefaults) then
    xrdb -nocpp -override ~/.Xdefaults
  endif
  # set fonts
  xset fp= /usr/share/X11/fonts/misc/,/usr/share/X11/fonts/75dpi/,/usr/share/X11/fonts/100dpi/
  xset fp= /usr/share/fonts/X11/misc/,/usr/share/fonts/X11/75dpi/,/usr/share/fonts/X11/100dpi/
  xset fp= /usr/X11R6/lib/X11/fonts/misc/,/usr/X11R6/lib/X11/fonts/75dpi/,/usr/X11R6/lib/X11/fonts/100dpi/
  # start CIDD
  /usr/local/lrose/bin/CIDD -font fixed -p http://front.eol.ucar.edu/displayParams/CIDD.pecan
```

This should download data remotely from NCAR/EOL, and display it on CIDD.

