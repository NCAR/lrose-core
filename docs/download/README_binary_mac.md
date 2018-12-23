## Install an LROSE binary release (Mac)

[Make sure your environment is ready for installation.](./dev/README_OSX_PREPARE_ENVIRONMENT.md)

### Choose your install directory (prefix location)

The default is: `/usr/local/lrose`

### Download the LROSE binary release

Create a directory for the release:

```
  mkdir release
  cd release
```

Download the binary tar file from:

```
  https://github.com/NCAR/lrose-core/releases
```


A typical binary release would be:

```
  lrose-20180430.bin.mac_osx.tgz
```

### Untar the release

```
  cd release
  tar xvfz lrose-20180430.bin.x86_64.tgz
```

The release will be unpacked into a subdirectory:

```
  release/lrose-20180430.bin.x86_64
```

So go there:

```
  cd release/lrose-20180430.bin.x86_64
```

### Installing

If you run:

```
  ./install_bin_release.py
```

it will install into the default location:

```
  /usr/local/lrose/bin
```


### Troubleshooting

#### dyld: Library not loaded: /usr/local/opt/netcdf/lib/libnetcdf.13.dylib. Referenced from: /usr/local/lrose/bin/RadxPrint Reason: image not found

Make sure netcdf is installed
```
brew install netcdf
```
