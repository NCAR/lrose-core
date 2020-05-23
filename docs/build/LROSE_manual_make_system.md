# The LROSE manual make system

## Introduction

LROSE depends on ```make``` and the associated ```Makefiles```.

On Unix-type systems (LINUX, OSX) running the compiler is most commonly managed by the ```make``` application.
 
```make``` uses configuration files to decide what to do. These are named one of the following:

* ```Makefile```
* ```makefile```

If both ```Makefile``` and ```makefile``` are present, the lower-case version takes precedence.

In LROSE, ```Makefile``` is the primary name, and these files are checked in permanently in git. Various procedures you can run will cause a ```makefile``` to be written to a directory, which will then override the ```Makefile```.

There are many tutorials on-line for Makefiles. For example see:

* [Makefile Tutorial](https://www.tutorialspoint.com/makefile/makefile_macros.htm)

## Environment variables

The LROSE manual make procedures depend on the following environment variables:

| Environment variable | Usage |
| ------------- |:--------:|
| **HOST_OS** | Flavor of the HOST OS. See [host_os](#host_os) |
| **LROSE_CORE_DIR** | Top-level directory of lrose-core, checked out from git |
| **LROSE_INSTALL_DIR** | Target location of installation. Will contain ```include, lib, bin``` |

## Anatomy of an LROSE Makefile

### Makefile elements

A Makefile contains several types of information:

  * macros - these store values that are used elsewhere
  * targets - what is to be built
  * rules - on how to build targets
  * suffix rules - automatic rules depending in the suffix of a file

### LROSE Makefile includes

The LROSE Makefiles are relatively short. The complexity is added by including partial makefiles, each with a specific purpose.

These partial makefiles reside in the directory:

* [lrose-core/build/make_include](../../build/make_include)

The following table lists the common top-level includes:

| Include file  | Purpose      |
| ------------- |:-------------:|
| [lrose_make_macros](../../build/make_include/lrose_make_macros) | main macro definitions |
| [lrose_make_targets](../../build/make_include/lrose_make_targets) | general target rules |
| [lrose_make_suffixes](../../build/make_include/lrose_make_suffixes) | suffix rules |
| [lrose_make_lib_targets](../../build/make_include/lrose_make_lib_targets) | targets for C libraries |
| [lrose_make_c_targets](../../build/make_include/lrose_make_c_targets) | targets for C apps |
| [lrose_make_c++_targets](../../build/make_include/lrose_make_c_targets) | targets for C++ apps |
| [lrose_make_qt_targets](../../build/make_include/lrose_make_qt_targets) | extra targets for QT apps |

### Macros for specific OS versions

In [lrose_make_macros](../../build/make_include/lrose_make_macros), you will see the following line:

```
include $(LROSE_CORE_DIR)/build/make_include/lrose_make.$(HOST_OS)
```

This includes a file that defines macros specific to the OS you are running. For this to work, you need to set the **HOST_OS** environment variable.

The common OS versions supported, along with the include files, are listed in the following table:

<a name="host_os"/>

| HOST_OS  | Include file       | Comments |
| ------------- |:-------------:|:--------:|
| LINUX_LROSE | [lrose_make.LINUX_LROSE](../../build/make_include/lrose_make.LINUX_LROSE) | normal LINUX build |
| OSX_LROSE | [lrose_make.OSX_LROSE](../../build/make_include/lrose_make.OSX_LROSE) | build on Mac OSX |
| CIDD_32 | [lrose_make.CIDD_32](../../build/make_include/lrose_make.CIDD_32) | 32-bit build for CIDD on LINUX |

The ```CIDD``` display application is dependent on the ```xview``` library, which will only work properly if built in 32-bit mode.

## LROSE Makefile templates

The actual Makefiles are created by filling out elements in a template. As mentioend above, the complexity is added by including partial makefiles.

The following template is for a library code subdirectory:

```
  # main macros
  include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros
  # local macros
  LOC_INCLUDES =
  LOC_CFLAGS =
  # target - library .a file
  TARGET_FILE =
  # list of headers
  HDRS =
  # list of C sources
  SRCS =
  # list of C++ sources
  CPPC_SRCS =
  # list of FORTRAN sources
  F_SRCS =
  # general targets
  include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets
```

For library code examples, see:

* [codebase/libs/dataport/src/bigend/Makefile](../../codebase/libs/dataport/src/bigend/Makefile)
* [codebase/libs/toolsa/src/pjg/Makefile](../../codebase/libs/toolsa/src/pjg/Makefile)
* [codebase/libs/Mdv/src/Mdvx/Makefile](../../codebase/libs/Mdv/src/Mdvx/Makefile)

The following template is for an application code directory:

```
  # include main macros
  include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros
  # main target - application name
  TARGET_FILE =
  # local macros
  LOC_INCLUDES =
  LOC_LIBS =
  LOC_LDFLAGS =
  LOC_CFLAGS =
  # header code files
  HDRS =
  # list of C sources
  SRCS =
  # list of C++ sources
  CPPC_SRCS =
  # list of FORTRAN sources
  F_SRCS =
  # tdrp macros
  include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros
  # C++ targets
  include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets
  # tdrp targets
  include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets
```

For application examples, see:

* [codebase/apps/Radx/src/RadxConvert/Makefile](../../codebase/apps/Radx/src/RadxConvert/Makefile)
* [codebase/apps/mdv_utils/src/PrintMdv/Makefile](../../codebase/apps/mdv_utils/src/PrintMdv/Makefile)
* [codebase/apps/radar/src/HawkEye/Makefile](../../codebase/apps/radar/src/HawkEye/Makefile)

The HawkEye example is more complicated, because it is a QT application, so we need to handle the QT complexities.

## Recursion through the code tree

LROSE sets up Makefiles at all levels of the code tree, both for the libraries and applications.
Except for the lowest level, where the actual code files reside, the Makefiles handle recursion to lower levels in the code tree.

As an example, for the dataport library, we have the following, from the top level to the bottom level:

* [codebase/libs/Makefile](../../codebase/libs/Makefile)
* [codebase/libs/dataport/Makefile](../../codebase/libs/dataport/Makefile)
* [codebase/libs/dataport/src/Makefile](../../codebase/libs/dataport/src/Makefile)
* [codebase/libs/dataport/src/bigend/Makefile](../../codebase/libs/dataport/src/bigend/Makefile)

Similarly, for the RadxConvert application, we have the following, from the top level to the bottom level:

* [codebase/apps/Makefile](../../codebase/apps/Makefile)
* [codebase/apps/Radx/Makefile](../../codebase/apps/Radx/Makefile)
* [codebase/apps/Radx/src/Makefile](../../codebase/apps/Radx/src/Makefile)
* [codebase/apps/Radx/src/RadxConvert/Makefile](../../codebase/apps/Radx/src/RadxConvert/Makefile)

## Installing package-specific makefiles

The Makefiles checked in under the LROSE code tree are applicable for the standard lrose-core build. We refer to this as the **lrose-core** package.

For some purposes, it is desirable to build a different package. For example, we need to be able to build the **lrose-cidd** package, which includes the ```CIDD``` display and other display utilities that depend on the xview library, which only works properly in 32-bit mode.

To support package-specific builds, we have a series of special-purpose directories nameed ```_makefiles```. These contain package-specific lower-case ```makefiles```.

As an example, for the **lrose-cidd** package, the following makefiles reside in the code tree:

```
  codebase/libs/_makefiles/makefile.lrose-cidd
  codebase/apps/dsserver/src/_makefiles/makefile.lrose-cidd
  codebase/apps/_makefiles/makefile.lrose-cidd
  codebase/apps/tdrp/src/_makefiles/makefile.lrose-cidd
  codebase/apps/scripts/src/_makefiles/makefile.lrose-cidd
  codebase/apps/procmap/src/_makefiles/makefile.lrose-cidd
  codebase/apps/radar/src/_makefiles/makefile.lrose-cidd
  codebase/apps/cidd/src/_makefiles/makefile.lrose-cidd
```

If we wish to build a specific package, we first run the ```installPackageMakefiles.py``` script.

This script resides in ```lrose-core/build/scripts```.

The usage is:

```
  installPackageMakefiles.py --help
  Usage: installPackageMakefiles.py [options]
  Options:
    -h, --help         show this help message and exit
    --debug            Set debugging on
    --osx              Configure for MAC OSX
    --package=PACKAGE  Name of distribution for which we are building
```

To set up the build for the **lrose-cidd** package, we would run:

```
  installPackageMakefiles.py --package lrose-cidd --debug
```

That will copy the various instances of ```makefile.lrose-cidd``` up one directory to the parent level, and change the name to the lower-case ```makefile```.
  
The effect would be as follows:

```
  cp codebase/libs/_makefiles/makefile.lrose-cidd codebase/libs/makefile
  cp codebase/apps/dsserver/src/_makefiles/makefile.lrose-cidd codebase/apps/dsserver/src/makefile.lrose-cidd
  cp codebase/apps/_makefiles/makefile.lrose-cidd codebase/apps/makefile
  cp codebase/apps/tdrp/src/_makefiles/makefile.lrose-cidd codebase/apps/tdrp/src/makefile
  cp codebase/apps/scripts/src/_makefiles/makefile.lrose-cidd codebase/apps/scripts/src/makefile
  cp codebase/apps/procmap/src/_makefiles/makefile.lrose-cidd codebase/apps/procmap/src/makefile
  cp codebase/apps/radar/src/_makefiles/makefile.lrose-cidd codebase/apps/radar/src/makefile
  cp codebase/apps/cidd/src/_makefiles/makefile.lrose-cidd codebase/apps/cidd/src/makefile
```

These lower-case ```makefiles``` will then take precedence over the uppercase ```Makefiles``` that exist at those locations. The build will use these lower-case versions instead.

The exception is on Mac OSX, which for odd reasons has a filesystem that is not properly case-sensitive. On OSX, ```installPackageMakefiles.py --osx```
will copy to upper-case ```Makefiles``` instead, overwriting the Makefiles that already exist at those parent locations. So if you run this procedure on OSX, make sure you do not check the resulting files into git, otherwise you will overwrite the **lrose-core** Makefiles.

To revert to the correct makefiles for **lrose-core**, you can always run:

```
  installPackageMakefiles.py --package lrose-core --debug
```


