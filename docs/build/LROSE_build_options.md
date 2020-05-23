## Building LROSE

### Available package builds

LROSE has the following package options:

| Package       | Comments      |
| ------------- |:-------------:|
| lrose-core    | standard full core package - the default |
| lrose-blaze   | blaze release - 2018 - tested and documented |
| lrose-cyclone | cyclone release - 2019 - tested and documented |
| cidd          | CIDD display apps only, 32-bit build |

`lrose-core` is the standard build, which includes all of the libraries and applications in lrose, except for the `cidd` display and its related applications.

`lrose-blaze` is the first of the offical releases from the NSF SI2 LROSE project. Calendar yeat 2018.
 
`lrose-cyclone` is the second of the offical releases from the NSF SI2 LROSE project. Calendar year 2019.

`cidd` is a special package, that must be built using 32-bit emulation, because the applications are based on the `xview` library that has no 64-bit port. This package includes the CIDD display, and other applications that depend on `xview`.

Note that starting in 2020, with the Elle release, the release uses the full lrose-core set of applictions.

### Options for building LROSE on LINUX

There are three ways to build LROSE:

1. Check out lrose-core from GitHub, and use the LROSE manual make system.
This is recommended if you are actively involved in developing the code.
See [README_NCAR_BUILD.md](./README_NCAR_BUILD.md) for details

2. Check out the source from GitHub, and use AUTOMAKE and CONFIGURE for the build.
This is the standard approach.
See [README_AUTOMAKE_BUILD.md](./README_AUTOMAKE_BUILD.md) for details

3. Download a pre-configured source distribution, and build from that.
See [README_DOWNLOAD_BUILD.md](./README_DOWNLOAD_BUILD.md) for details

### Building CIDD on LINUX

To build CIDD, see:
[README_CIDD.md](./README_CIDD.md).

### Building LROSE on MAC OSX

See:
[README_OSX_BUILD.md](./README_OSX_BUILD.md).

### Supported operating systems

LROSE was developed and tested extensively under LINUX.

Therefore LINUX is the preferred operating system.

However, LROSE can be compiled and run under Mac OSX.

Windows is supported using a Docker container, or the [Windows Subsystem For Linux](https://docs.microsoft.com/en-us/windows/wsl/install-win10)

### Required LINUX and gcc/g++ versions for LROSE build

Most good, up-to date LINUX distributions should work.

Recommended distributions are:

  * Debian
  * Ubuntu
  * RedHat
  * Centos
  * Fedora
  * Opensuse

### gcc/g++ versions for LROSE build

LROSE expects support for the c++11 standard.

The gcc/g++ version should be 4.8.5 or later.

### Required LINUX packages for the LROSE build

See: [LROSE package dependencies](./lrose_package_dependencies.md)

### Required LINUX packages for the CIDD build

See: [README_CIDD.md](./README_CIDD.md).

