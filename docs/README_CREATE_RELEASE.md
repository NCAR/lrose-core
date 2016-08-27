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

The release will be saved as:

```
  releaseDir/package-yyyymmdd.src.tgz
  package.rb
```

where `package` is the package name, and `package.rb` is the OSX brew recipe file.

