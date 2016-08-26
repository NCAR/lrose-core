# lrose-netcdf

### Choosing the prefix - i.e. the install directory

The default install location for LROSE is:

```
    /usr/local/lrose
```

### Check out, build and install **netcdf** support

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

