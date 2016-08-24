# Building using AUTOMAKE and CONFIGURE

### Chose your install location

The default is: `/usr/local/lrose`

### Check out, build and install netcdf support

The default install:

```
  git clone https://github.com/NCAR/lrose-netcdf
  cd lrose-netcdf
  ./build_and_install_netcdf
```

or, say, installing in `/tmp/mybuild`:

```
  ./build_and_install_netcdf /tmp/mybuild
```
### Check out LROSE

```
  git clone https://github.com/NCAR/lrose-core
  cd lrose-core
```

### Install the makefile tree

The `make` application can use files named either `Makefile` or `makefile`.

The lower-case version takes preference.

The codebase contains, by default, upper-case Makefiles throughout the tree. These are **NOT** appropriate for the build.

To get the correct build, you must install the lower-case makefiles relevant to the package you want to build.

The following are the package options:

| Package       | Comments      |
| ------------- |:-------------:|
| lrose         | standard full build - the default |
| radx          | Radx apps only |
| hcr           | HCR (Hiaper Cloud Radar) build |
| cidd          | CIDD display apps only, 32-bit build |

To install the makefiles for the **lrose** standard package, perform the following:

```
  cd lrose-core/codebase
  ./make_bin/install_package_makefiles.py
```
This is equivalent to the following

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_package_makefiles.py --package lrose
```

If you want to perform a partial build for a sub distribution, you can specify that on the command line.

For the **radx** distribtion, run the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_package_makefiles.py --package radx
```

For the **cidd** distribtion, run the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_package_makefiles.py --package cidd
```

For the **hcr** distribtion, run the following:

```
  cd $LROSE_CORE_DIR/codebase
  ./make_bin/install_package_makefiles.py --package hcr
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

### Running configure

First, go to the codebase directory, and run configure:

```
  cd lrose-core/codebase
  ./configure --prefix=/tmp/lrose
```

or whatever install location you want.

### Performing the build

#### Building and installing the libraries

```
  cd lrose-core/codebase/libs/
  make -j 8 install
```

#### Building and installing the applications

```
  cd lrose-core/codebase/apps
  make -j 8 install
```

