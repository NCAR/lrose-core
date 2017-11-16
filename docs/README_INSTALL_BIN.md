## INSTALLING an LROSE binary release

### Choose your install directory (prefix location)

The default is: `${HOME}/lrose`

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
  lrose-20160823.x84_64.tgz
```

### Untar the release

```
  cd release
  tar xvfz lrose-20160823.x86_64.tgz
```

The release will be unpacked into a subdirectory:

```
  release/lrose-20160823.x86_64
```

Any suitable directory can be used for this purpose.

### Installing binaries only

You will probably need to be root for this step, unless you install
in your user area.

```
  cd release/lrose-20160823.x86_64
  ./install_bin_release 
```

will install into

```
  ${HOME}/lrose/bin
```

You can specify an alternative, for example:

```
  ./install_bin_release /usr/local/lrose
```

will install into

```
  /usr/local/lrose/bin
```

The run-time dynamic libraries will be found in:

```
  ..../bin/lrose_runtime_libs
```

i.e. in a subdirectory of the bin directory.

### Installing development release

If you want to compile and build against the LROSE libraries, you will need the
development release which has the library and include files in addition to the
binary files.

To do this, use

```
  cd release/lrose-20160823.x86_64
  ./install_devel_release
```

This will install in:

```
  ${HOME}/lrose/bin
  ${HOME}/lrose/lib
  ${HOME}/lrose/include
```

Or, for example:

```
  ./install_devel_release /usr/local/lrose
```

will install in:

```
  /usr/local/lrose/bin
  /usr/local/lrose/lib
  /usr/local/lrose/include
```


