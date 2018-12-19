## CREATING an LROSE release

### Available packages

LROSE has the following package options:

| Package       | Comments      |
| ------------- |:-------------:|
| lrose         | standard full package - the default |
| lrose-blaze   | blaze release - tested and documented |
| radx          | Radx apps only |
| titan         | Titan distribution |
| cidd          | CIDD display apps only, 32-bit build |

`lrose` is the standard build, which includes all of the libraries and applications in lrose, except for the `cidd` display and its related applications.

`lrose-blaze` is the first of the offical releases from the NSF SI2 LROSE project.

`radx` is a sub package that only includes the `Radx` applications.

`titan` is a sub package that supercedes the old Titan distribution for applications.

`cidd` is a special package, that must be built using 32-bit emulation, because the applications are based on the `xview` library that has no 64-bit port. This package includes the CIDD display, and other applications that depend on `xview`.

### Creating a source release

By default a source release will be saved in:

```
  $HOME/releases/package_name
```

To create the release, you run the script:
```
  create_src_release.py
```

The usage is:

```
Usage: create_src_release.py [options]

Options:
  -h, --help            show this help message and exit
  --debug               Set debugging on
  --verbose             Set verbose debugging on
  --osx                 Configure for MAC OSX
  --package=PACKAGE     Package name. Options are lrose, lrose-blaze, radx,
                        cidd, titan
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

The release will be saved as:

```
  releaseDir/package-yyyymmdd.src.tgz
  package.rb
```

where `package` is the package name, and `package.rb` is the OSX brew recipe file.

### Creating a binary release

You create a binary release from a source release.

Generally this is done using the /tmp directory.

We will use the example of the `lrose-20160827.src.tgz` source release.

```
  cd /tmp
  cp -r ~/releases/lrose/lrose-20160827.src.tgz .
  tar xvfz lrose-20160827.src.tgz
  cd lrose-20160827.src
  ./create_bin_release.py
```

The `create_bin_release.py` script is installed at the top level of the release.

The usage is:

```
Usage: create_bin_release.py [options]

Options:
  -h, --help            show this help message and exit
  --debug               Set debugging on
  --verbose             Set verbose debugging on
  --prefix=PREFIX       Prefix name for install location
  --releaseDir=RELEASETOPDIR
                        Top-level release dir
  --force               force, do not request user to check if it is OK to
                        proceed
```

The `prefix` refers to the directory into which the binary release will be installed as it is prepared. This defaults to:

```
  /tmp/{package}_prepare_release_bin_directory
```

and generally this need not be changed from the default. The binary release is prepared in that directory, before being collected into the final tar file.

By default a binary release will be saved in:

```
  $HOME/releases/package_name
```

The release will be saved as:

```
  releaseDir/package-yyyymmdd.bin.x86_64.tgz
```

for a 64-bit LINUX build.

Here `package` is the package name.

For a 32-bit build the name will be:
```
  releaseDir/package-yyyymmdd.bin.i686.tgz
```

For a Mac OSX 64-bit build the name will be:
```
  releaseDir/package-yyyymmdd.bin.mac_osx.tgz
```




