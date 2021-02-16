# BUILDING LROSE

## CMAKE builds for scientific users

cmake works well on both LINUX and MAC OSX.

These are the recommended builds, if you just want working apps and cannot get the normal installs to work.

| Target | Notes  |
| ---------- |:------:|
| [lrose-core](./LROSE_cmake_build.md) | Build using cmake from lrose-bootstrap repo |
| [fractl](./build_fractl.md) | Build fractl using cmake |
| [vortrac](./build_vortrac.md) | Build vortrac using cmake |
| [samurai](./build_samurai.md) | Build samurai using cmake |

## Autoconf builds

If for some reason cmake does not work, you can try the autoconf build.
Also, autoconf is used for building CIDD, and NetCDF if required.
If NetCDF does not properly install on your system, you can build it instead.

| Target | Notes  |
| ---------- |:------:|
| [lrose-core](./LROSE_autoconf_build.linux.md) | Build using autoconf from lrose-bootstrap repo |
| [CIDD](./CIDD_build.linux.md) | Build CIDD in 32-bit emulation mode. LINUX only |
| [NetCDF](./NETCDF_build.linux.md) | Build HDF5 and NetCDF |

## Package dependencies

| Target | Operating System | Description |
| ------ |:------------:|:------------:|
| [lrose-core](./lrose_package_dependencies.linux.md) | LINUX | Full core |
| [lrose-core](./lrose_package_dependencies.osx.md) | MAC OSX | Full core |
| [lrose-cidd](./lrose_package_dependencies.cidd.md) | LINUX | CIDD display |

## Code development

Use the following if you need to set up an environment suitable for code development.

| Topic | Description |
| ------ |:------------:|
| [Build overview](./LROSE_build_overview.md) | Overview of the LROSE build systems |
| [LROSE make system](./LROSE_manual_make_system.md) | Details of the manual make system |
| [Build manually - LINUX](./LROSE_manual_build.linux.md) | Manual build for LROSE, for LINUX |
| [Build manually - MAC](./LROSE_manual_build.osx.md) | Manual build for LROSE, for MAC OSX |

