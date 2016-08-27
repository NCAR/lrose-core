## CREATING an LROSE release

### Available packages

You can create an LROSE release for the following packages:

| Package       | Comments      |
| ------------- |:-------------:|
| lrose         | standard full package - the default |
| radx          | Radx apps only |
| hcr           | HCR (HIAPER Cloud Radar) package |
| cidd          | CIDD display apps only, 32-bit build |

`lrose` is the standard build, which includes all of the libraries and applications in lrose, except for the `cidd` display and its related applications.

`radx` is a sub package that only includes the `Radx` applications.

`hcr` is a sub package that only includes the applications required for the HIAPER Cloud Radar.

`cidd` is a special package, that must be built using 32-bit emulation, because the applications are based on the `xview` library that has no 64-bit port. This package includes the CIDD display, and other applications that depend on `xview`.

### Creating a source release

By default a source release will be saved in:

```
  $HOME/releases/package_name
```

To create the release, you run the script:
```
  create_src_release
```

The usage is:

```
Usage: create_src_release.py [options]

Options:
  -h, --help            show this help message and exit
  --debug               Set debugging on
  --verbose             Set verbose debugging on
  --package=PACKAGE     Package name. Options are lrose (default), radx, cidd,
                        hcr
  --releaseDir=RELEASETOPDIR
                        Top-level release dir
  --force               force, do not request user to check it is OK to
                        proceed
  --static              produce distribution for static linking, default is
                        dynamic
```

Specify the package on the command line. The default is `lrose`.

By default the build will support dynamic linking. Use `--static` to specify static linking.

You can override the release directory location using `--releaseDir`.



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
  /usr/local/bin
```

You can specify an alternative, for example:

```
  ./install_bin_release /opt/local
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
  cd release/lrose-20160823.x86_64
  ./install_devel_release
```

This will install in:

```
  /usr/local/bin
  /usr/local/lib
  /usr/local/include
```

Or, for example:

```
  ./install_devel_release /opt/local
```

will install in:

```
  /opt/local/bin
  /opt/local/lib
  /opt/local/include
```


