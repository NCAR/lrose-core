# Building using AUTOMAKE and CONFIGURE

### Choose your install location

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

If you are building the CIDD package, this needs a 32-bit emulation build:

```
  ./build_and_install_netcdf.m32 /tmp/cidd_m32
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
  ./make_bin/install_package_makefiles.py --package lrose
```

If you want to perform a partial build for a sub distribution, you can specify that on the command line.

As an example, for the **radx** distribtion, run the following:

```
  ./make_bin/install_package_makefiles.py --package radx
```

### Running autoconf

First, go to the top-level direectory: `lrose-core`

Then run:

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

where prefix points to the install location, most commonly `/usr/local/lrose`.

### Performing the build

Got to the codebase directory, and run the make:

```
  cd lrose-core/codebase
  make -j 8 install
```
