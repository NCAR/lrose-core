# Building lrose-netcdf

### Choose the prefix - i.e. the install directory

The default install location for LROSE is:

```
    ${HOME}/lrose
```

You can specify the prefix with the ```-x``` command line argument.

### Check out, build and install **netcdf** support

The following is the default:

```
  git clone https://github.com/NCAR/lrose-netcdf
  cd lrose-netcdf
  ./build_and_install_netcdf
```

and will build and install netcdf in `${HOME}/lrose`

For, say, installing in `/usr/local/lrose`:

```
  ./build_and_install_netcdf -x /usr/local/lrose
```

### MAC OSX build

For OSX, you will install NetCDF using brew.
So the build step is generally not necessary.

If you do want to build a specific copy of NetCDF:

```
  git clone https://github.com/NCAR/lrose-netcdf
  cd lrose-netcdf
  ./build_and_install_netcdf.osx -x /usr/local/lrose
```

will install NetCDF and HDF5 in:

```
  /usr/local/lrose
```

