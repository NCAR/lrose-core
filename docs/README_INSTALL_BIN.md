## INSTALLING an LROSE binary distribution

### Choose your install directory (prefix location)

The default is: `/usr/local/lrose`

### Download the LROSE binary distribution

Create a directory for the distribution:

```
  cd
  mkdir dist
  cd dist
```

Download the binary tar file from:

```
  https://github.com/NCAR/lrose-core/releases
```

For a 64-bit LINUX system, a typical binary release would be:

```
  lrose-20160823.x84_64.tgz
```

### Untar the distribution

```
  cd lrose_build
  tar xvfz lrose-20160823.x86_64.tgz
```

The distribution will be unpacked into a subdirectory:

```
  dist/lrose-20160823.x86_64
```

Download the file into a tmp area. For example:

  /tmp/lrose_dist

and install from there.

However, any suitable directory can be used for this purpose.

### Installing binaries only

You will probably need to be root for this step, unless you install
in your user area.

```
  cd dist/lrose-20160823.x86_64
  install_bin_release 
```

will install into

```
  /usr/local/bin
```

You can specify an alternative, for example:

```
  install_bin_release /opt/local
```

will install into

```
  /opt/local/bin
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
  install_devel_release
```

This will install in:

```
  /usr/local/bin
  /usr/local/lib
  /usr/local/include
```

Or, for example:

```
  install_devel_release /opt/local
```

will install in:

```
  /opt/local/bin
  /opt/local/lib
  /opt/local/include
```


