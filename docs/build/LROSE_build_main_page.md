# BUILDING LROSE

## CMAKE builds for scientific users

cmake works well on both LINUX and MAC OSX.

These are the recommended builds, if you just want working apps and cannot get the normal installs to work.

| Target | Notes  |
| ---------- |:------:|
| [lrose-core](./LROSE_cmake_build.auto.md) | Build lrose-core autonomously using cmake |
| [lrose-core](./LROSE_cmake_build.manual.md) | Build lrose-core manually using cmake |
| [fractl](./build_fractl.md) | Build fractl using cmake |
| [vortrac](./build_vortrac.md) | Build vortrac using cmake |
| [samurai](./build_samurai.md) | Build samurai using cmake |

## Autoconf builds

If for some reason cmake does not work for lrose-core, you can try the autoconf build.

Also, autoconf is used for building CIDD, and NetCDF if required.

If the system install for NetCDF on your system does not work well, you can build it instead.
It should be installed with the same prefix as you plan to use for the core.

| Target | Notes  |
| ---------- |:------:|
| [lrose-core](./LROSE_autoconf_build.linux.md) | Build using autoconf |
| [CIDD](./CIDD_build.linux.md) | Build CIDD in 32-bit emulation mode. LINUX only |
| [NetCDF](./NETCDF_build.linux.md) | Build HDF5 and NetCDF |

## Manual builds and code development

Use the following if you need to set up an environment suitable for code development.

| Topic | Description |
| ------ |:------------:|
| [LROSE make system](./LROSE_manual_make_system.md) | Details of the manual make system |
| [Manual build - LINUX](./LROSE_manual_build.linux.md) | Manual build for LROSE, for LINUX |
| [Manual build - MAC OSC](./LROSE_manual_build.osx.md) | Manual build for LROSE, for MAC OSX |
| [Manual build - Windows](https://github.com/NCAR/lrose-core/files/6736210/LROSE.Windows.Installation.Help.pdf) | Manual build for LROSE, for Windows, using WSL |

## Creating .rpm and .deb package files

See: [Building package files](https://github.com/NCAR/lrose-bootstrap/blob/main/docker/README.md)

## Package dependencies

| Target | Operating System | Description |
| ------ |:------------:|:------------:|
| [lrose-core](./lrose_package_dependencies.linux.md) | LINUX | Full core |
| [lrose-core](./lrose_package_dependencies.osx.md) | MAC OSX | Full core |
| [lrose-cidd](./lrose_package_dependencies.cidd.md) | LINUX | CIDD display |

