# Building CIDD using AUTOMAKE and CONFIGURE

### Choose your install directory (prefix)

The CIDD display uses the xview library, which must be compiled with 32-bit emulation.

**It is important** to keep the **cidd** build/install separate from the main build,
so that you do not mix 32-bit object files with 64-bit executables.

So, for a CIDD build we recommend you use a temporary prefix location, for example:

  `/tmp/cidd_m32`

After the package has been built into this temporary location, it can be copied into the final location.

### Check out, build and install netcdf support

Build and install netcdf into the temporary build area:

```
  git clone https://github.com/NCAR/lrose-netcdf
  cd lrose-netcdf
  ./build_and_install_netcdf.m32 /tmp/cidd_m32
```

This will install in `/tmp/cidd_m32`

### Check out LROSE

```
  git clone https://github.com/NCAR/lrose-core
  cd lrose-core
```

### Install the makefile tree

The `make` application can use files named either `Makefile` or `makefile`.

The lower-case version takes preference.

The codebase contains, by default, upper-case Makefiles throughout the tree. These are **NOT** appropriate for the cidd build.

To get the correct build, you must install the lower-case makefiles relevant to the package you want to build.

To install the makefiles for the **cidd** package, perform the following:

```
  ./make_bin/install_package_makefiles.py --package cidd
```

### Performing the build

Build using automake:

```
  cd lrose-core
  ./build/make_auto -p cidd -x /tmp/cidd_m32
```

### Copy the binaries to the final install location

Use rsync to copy the binaries to the final location.

For example:

```
  rsync -av /tmp/cidd_m32/bin /usr/local/lrose
```

The final install will be in:

```
  /usr/local/lrose/bin
```

