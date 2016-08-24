# Building using AUTOMAKE and CONFIGURE

### Chose your install location

The default is:

```
  /usr/local/lrose
```
### Check out, build and install netcdf support

```
  git clone https://github.com/NCAR/lrose-netcdf
  cd lrose-netcdf
  ./build_and_install_netcdf
```

### Check out LROSE

```
  git clone https://github.com/NCAR/lrose-core
  cd lrose-core
```

### Install the makefile tree

The `make` application can use files named either `Makefile` or `makefile`.

The lower-case version takes preference.

The codebase contains, by default, upper-case Makefiles throughout the tree. These are NOT appropriate for the build.

To get the correct build, you must install the lower-case makefiles relevant to the distribution you need.

The following are the build makefile options:

| Name          | Comments      |
| ------------- |:-------------:|
| lrose         | full build    |
| radx          | Radx apps only |
| cidd          | CIDD apps only, 32-bit build |
| hcr           | HCR (Hiaper Cloud Radar) build |

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

