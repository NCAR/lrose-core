# Building netcdf for NCAR LROSE development

## NOTE:

Generally this is no longer needed, and we use the
system-installed Hdf5 and NetCDF libs.

## To install NetCDF

Follow these steps to build HDF5 and NetCDF support for the
NCAR LROSE development and build environment.

## Choose the prefix - i.e. the install directory

The default install location for LROSE is:

```
    ${HOME}/lrose
```

You can specify the prefix with the ```-x``` command line argument.

## Check out, build and install **netcdf** support

The following is the default:

```
  mkdir -p ~/git
  cd ~/git
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

brew will install netcdf and hdf5 in /usr/local/opt.

If you do want to build a specific copy of NetCDF, do so
in the user's home directory so it does not conflict with
the version installed by homebrew.

```
  git clone https://github.com/NCAR/lrose-netcdf
  cd lrose-netcdf
  ./build_and_install_netcdf.osx -x ~/lrose
```

will install NetCDF and HDF5 in:

```
  ~/lrose
```

