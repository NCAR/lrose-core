### How do I install LROSE?  I don't need to modify the source code; I just want to run the software.

Install the binary version of the lrose release from 20180626.  You can just use the binary version.  

Here are the steps I used to install  ...
```
     cd ~/Downloads
     wget https://github.com/NCAR/lrose-core/releases/download/lrose-20180626/lrose-20180626.bin.x86_64.tgz
     tar xvfz lrose-20180626.bin.x86_64.tgz
     ls
     cd lrose-20180626.bin.x86_64
     ./install_bin_release.py --verbose --prefix = ~/lrose
     ~/lrose/bin/RadxPrint -h
     ~/lrose/bin/RadxPrint -h
     ~/lrose/bin/Titan -h
```

### I can use RadXConvert, but not HawkEye
```
RadxConvert -f Documents/LROSE/mydata.nc -outdir Documents/LROSE/OUT/
======================================================================
Program 'RadxConvert'
Run-time 2019/07/16 14:16:20.

HawkEye -f Documents/LROSE/OUT/20190716/
HawkEye: error while loading shared libraries: libQt5Qml.so.5: cannot open shared object file: No such file or directory
```

The error is caused by a missing library that handles Java Scripting.  HawkEye uses a Java Script editor to edit data, and applying Solo functions to the cfradial data.  To fix the error, try installing the qt5-qtdeclarative-devel library as in one of these instances:

#### Adding QJSEngine, on centos install
```
yum install qt5-qtdeclarative-devel
```

#### on ubuntu/debian 
```
apt-get qtdeclarative5-dev
```

### [ 98%] Linking CXX executable build/release/bin/samurai
### /usr/local/lrose/lib/libNcxx.so: undefined reference to `H5::H5Location::createAttribute … 

The linker is trying to find the HDF5 libraries, but it cannot.  The installation scripts have changed to use the HDF5 libraries present on the computer instead of always installing an LROSE specific version.

Try these commands:

```
HDF5_LIBRARY=-lhdf5 cmake .
make
make install
```

