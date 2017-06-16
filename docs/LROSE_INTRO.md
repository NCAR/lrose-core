# INTRODUCTION TO LROSE CORE

## Primary links

| Package           | Comments      |
| -------------     |:-------------:|
| core              | https://github.com/NCAR/lrose-core |
| core docs         | https://github.com/NCAR/lrose-core/tree/master/docs |
| user docs         | https://github.com/NCAR/lrose-docs |
| netcdf support    | https://github.com/NCAR/lrose-netcdf |
| display support   | https://github.com/NCAR/lrose-displays |
| Matlab display    | https://github.com/NCAR/lrose-emerald |
| Java display      | https://github.com/NCAR/lrose-jazz |
| Legacy C display  | https://github.com/NCAR/lrose-soloii |

## Code organization

| libs       | https://github.com/NCAR/lrose-core/blob/master/docs/lrose-libs-summary.pdf |
| apps       | https://github.com/NCAR/lrose-core/blob/master/docs/lrose-apps-summary.pdf |

## TDRP

Most LROSE apps use TDRP - Table Driven Runtime Parameters - to handle the parameters
that govern how the app runs.

| lib       | https://github.com/NCAR/lrose-core/tree/master/codebase/libs/tdrp |
| apps      | https://github.com/NCAR/lrose-core/tree/master/codebase/apps/tdrp |

The most important TDRP app is `tdrp_gen`.

See the full TDRP docs at:

```
  https://github.com/NCAR/lrose-docs/tree/master/tdrp
```

## Running LROSE applications

LROSE is a derivation of the older TITAN distribution. TITAN is now included in LROSE.

LROSE apps are run in the same manner as the TITAN apps.

See:

```
  http://www.ral.ucar.edu/projects/titan/home/
  http://www.ral.ucar.edu/projects/titan/home/
```

and specifically:

```
  http://www.ral.ucar.edu/projects/titan/docs/TitanRunning.pdf
```

As a starting point, these give a good idea about the philosophy behind the
LROSE runtime environment.

We need to bring these docs over into LROSE, and update them accordingly.


### Available package builds

LROSE has the following package options:

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

### Options for building LROSE on LINUX

There are three ways to build LROSE:

1. Check out the source from GitHub, and use the NCAR build system.
This is recommended if you are actively involved in developing the code.
See [README_NCAR_BUILD.md](./README_NCAR_BUILD.md) for details
2. Check out the source from GitHub, and use AUTOMAKE and CONFIGURE for the build.
This is the more standard approach.
See [README_AUTOMAKE_BUILD.md](./README_AUTOMAKE_BUILD.md) for details
3. Download a pre-configured source distribution, and build from that.
See [README_DOWNLOAD_BUILD.md](./README_DOWNLOAD_BUILD.md) for details

