# Building using AUTOMAKE and CONFIGURE

### Available package builds

LROSE has the following package options:

| Package       | Comments      |
| ------------- |:-------------:|
| lrose         | standard full build - the default |
| radx          | Radx apps only |
| hcr           | HCR (Hiaper Cloud Radar) build |
| cidd          | CIDD display apps only, 32-bit build |

If you want to build the `cidd` package, see:

  [README_CIDD_BUILD.md](./README_CIDD_BUILD.md)

### Choose your install directory (prefix)

The default is: `/usr/local/lrose`

### Check out, build and install netcdf support

The following default:

```
  git clone https://github.com/NCAR/lrose-netcdf
  cd lrose-netcdf
  ./build_and_install_netcdf
```

will build and install netcdf in `/usr/local/lrose`

For, say, installing in `/tmp/mybuild`:

```
  ./build_and_install_netcdf -x /tmp/mybuild
```

### Check out LROSE

```
  git clone https://github.com/NCAR/lrose-core
  cd lrose-core
```

<!---
### Install the makefile tree

The `make` application can use files named either `Makefile` or `makefile`.

The lower-case version takes preference.

The codebase contains, by default, upper-case Makefiles throughout the tree. These are **NOT** appropriate for the build.

To get the correct build, you must install the lower-case makefiles relevant to the package you want to build.

To install the makefiles for the **lrose** standard package, perform the following:

```
  cd lrose-core/codebase
  ./make_bin/install_package_makefiles.py
```
This is equivalent to the following

```
  ./make_bin/install_package_makefiles.py --package lrose
```

If you want to perform a package-specific build, you can specify that on the command line.

As an example, for the **radx** distribtion, run the following:

```
  ./make_bin/install_package_makefiles.py --package radx
```

--->

### Performing the build

To build using automake:

```
  cd lrose-core
  ./build/build_auto -p package -x prefix
```

where `prefix` is the location into which you are building.

`package` defaults to `lrose`

`prefix` defaults to `/usr/local/lrose`

