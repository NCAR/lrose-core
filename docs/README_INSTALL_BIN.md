## INSTALLING an LROSE binary release

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

For a 64-bit LINUX system, a typical binary release would be:

```
  lrose-20180430.bin.x84_64.tgz
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
  ./install_bin_release.py --prefix ~/lrose
```

it will install into the default location:

```
  /usr/local/lrose/bin
```

You can specify where to perform the install:

```
  ./install_bin_release.py --prefix ~/lrose
```

will install into

```
  ${HOME}/lrose/bin
```

For LINUX, the dynamic run-time libraries will be found in:

```
  ..../bin/lrose_runtime_libs
```

i.e. in a subdirectory of the bin directory.

