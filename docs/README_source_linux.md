## Installing an LROSE Source Release (Linux)

1. [prepare](#prepare)
2. [install](#install)
3. [verify](#verify)

<a name="prepare"/>

### Choose an install directory (prefix location)

The default is: `/usr/local/lrose`

### Check out, build and install **netcdf** 

See [README_NETCDF_BUILD.md](./dev/README_NETCDF_BUILD.md)

Install netcdf into the prefix location.


<a name="install"/>

### Prepare build directory

Create a directory for the distribution:

```
  cd
  mkdir lrose_build
  cd lrose_build
```

### Download source release for Linux

Download the source tar file from:

```
  https://github.com/NCAR/lrose-core/releases
```

A typical source release would be:

```
  lrose-20160823.src.tgz
```

### Untar it

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
  ./build_lrose.py --prefix installDir
```

The default prefix is $HOME/lrose.

This will install in:

```
  installDir/include
  installDir/lib
  installDir/bin
```
<a name="verify"/>

### Verify the installation

The build checks are run automatically at the end of the build script.

However, you also can run the checks independently:

After the build, you can check the build as follows:

```
  ./installDir/bin/RadxPrint -h
  ./installDir/bin/RadxConvert -h
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

