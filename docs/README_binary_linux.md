## Install an LROSE binary release (Linux)


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
  ./install_bin_release.py
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

The dynamic run-time libraries will be found in:

```
  ..../bin/lrose_runtime_libs
```

i.e. in a subdirectory of the bin directory.

### Troubleshooting

#### This application failed to start because it could not find or load the Qt platform plugin "xcb" in "".  Reinstalling the application may fix this problem.

Make sure Qt5 is installed

yum install Qt5

