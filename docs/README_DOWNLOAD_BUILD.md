## LINUX build from downloaded source distribution

### Choose your install directory (prefix location)

The default is: `${HOME}/lrose`

### Check out, build and install **netcdf** support

See [README_NETCDF_BUILD.md](./README_NETCDF_BUILD.md)

Install into the chosen prefix location.

### Download LROSE

Create a directory for the distribution:

```
  cd
  mkdir lrose_build
  cd lrose_build
```

Download the source tar file from:

```
  https://github.com/NCAR/lrose-core/releases
```

A typical source release would be:

```
  lrose-20160823.src.tgz
```

### Untar the distribution

```
  cd lrose_build
  tar xvfz lrose-20160823.src.tgz
```

The distribution will be unpacked into a subdirectory:

```
  lrose_build/lrose-20160823.src
```

### Run the build scripts:

```
  cd lrose_build/lrose-20160823.src
  ./build_lrose -x prefix
```

This will install in:

```
  prefix/include
  prefix/lib
  prefix/bin
```

### Checking the build

The build checks are run automatically at the end of the build script.

However, you also can run the checks independently:

After the build, you can check the build as follows:

```
  ./build/check_libs -x prefix
  ./build/check_apps -x prefix
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

