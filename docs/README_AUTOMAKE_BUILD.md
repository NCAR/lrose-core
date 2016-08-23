# Building using AUTOMAKE and CONFIGURE

### Check out the code

git clone https://github.com/NCAR/lrose-netcdf
git clone https://github.com/NCAR/lrose-core

### Build and install HDF5 and NetCDF support

See https://github.com/NCAR/lrose-netcdf

Go into the lrose-netcdf distribution and build from there.

### Installing the makefile tree

First, go to the top-level 'lrose-core' directory.

The `make` application can use files named either `Makefile` or `makefile`.
The lower-case version takes preference.

The codebase is checked in with upper-case Makefiles throughout the tree. These are NOT appropriate for the build.

To get the correct build, you must install the lower-case makefiles relevant to the distribution you need.

To install the **lrose** standard distribution makefiles, perform the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py
```
This is equivalent to the following

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py --distro lrose
```

If you want to perform a partial build for a sub distribution, you can specify that on the command line.

For the **radx** distribtion, run the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py --distro radx
```

For the **cidd** distribtion, run the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py --distro cidd
```

For the **hcr** distribtion, run the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_distro_makefiles.py --distro hcr
```

### Running autoconf

First, go to the top-level

```
  lrose-core
```

directory. Then run:

```
  ./build/run_autoconf -p package_name
```

where package_name can be 'lrose' (the default), 'radx', 'cidd' or 'hcr'.

### Performing the build

#### Building and installing the libraries

```
  cd codebase/libs/
  make -j 8 install
```

#### Building and installing the applications

```
  cd codebase/apps
  make -j 8 install
```

