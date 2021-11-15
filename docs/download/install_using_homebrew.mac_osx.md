# Homebrew LROSE install - MAC OSX

1. [prepare](#prepare)
2. [download](#download)
3. [install](#install)
4. [verify](#verify)
5. [upgrade](#upgrade)
6. [fractl](#fractl)
7. [vortrac](#vortrac)
8. [samurai](#samurai)

<a name="prepare"/>

## 1. Prepare (requires admin privileges)

### Install XCode and the Command Line Tools

Install either the XCode development environment or a stand-alone version of the
XCode command line tools.  If you intend to do lots of Apple development and
want to use an IDE, then install XCode.

You also need to install Homebrew. See:

* [Install XCode and Homebrew](../build/lrose_package_dependencies.osx.md)

## 2. Download

**NOTE - when using brew, do NOT use admin privileges.**
**Always run brew as yourself, otherwise you will create permission problems.**

You need to download the brew formula from the lrose repository.
This formula is used to perform the homebrew build.

Download ```lrose-core.rb``` from:

* [https://github.com/NCAR/lrose-core/releases](https://github.com/NCAR/lrose-core/releases)

Choose the latest version.

<a name="install"/>

## 3. Install (DO NOT use admin privileges)

Let us assume you have downloaded the latest core distribution, containing all of the core apps.

```
  cd ~/Downloads
  brew install lrose-core.rb
```

Note that homebrew often prints warnings that are benign.
This is because we are using formulae rather than casks.
For example you should expect to see:

```
  Error: Failed to load cask: lrose-core.rb
  Cask 'lrose-core' is unreadable
  Warning: Treating lrose-core.rb as a formula.
```

While homebrew is building, it creates log files so you can track the progress.

You can ignore the following message:

```
  Warning: lrose-core dependency gcc was built with a different C++ standard
```

The location of the log files for the various distributions will be:

```
  ~/Library/Logs/Homebrew/lrose-core
  ~/Library/Logs/Homebrew/lrose-blaze
  ~/Library/Logs/Homebrew/lrose-cyclone
  ~/Library/Logs/Homebrew/radx
```

You will see the following log files:

```
  00.options.out
  01.configure.cc
  01.configure
  02.make
  02.make.cc
  03.make
  03.make.cc
  04.make
  04.make.cc
```

The log files contain the following:

* 02.make: building libs/tdrp
* 03.make: building apps/tdrp/src/tdrp_gen
* 04.make: building everything else

02.make contains the logs for building libs/tdrp.

You can watch the progress using:

```
  tail -f 01.configure
  tail -f 02.make
  tail -f 03.make
  tail -f 04.make
```

If the build is successful, lrose will be installed in:

```
  /usr/local/opt/lrose/include
  /usr/local/opt/lrose/lib
  /usr/local/opt/lrose/bin
```

In these directories, links will be created that point to the actual files
in ```/usr/local/Cellar```.

<a name="verify"/>

## 4. Verify the installation

Try the commands:
```
  /usr/local/opt/lrose/bin/RadxPrint -h
  /usr/local/opt/lrose/bin/RadxConvert -h
  /usr/local/opt/lrose/bin/Radx2Grid -h
  /usr/local/opt/lrose/bin/HawkEye
```

<a name="upgrade"/>

## 5. Upgrade to a new LROSE version (DO NOT use admin privileges)

When the time comes to upgrade, you will first need to uninstall the current version.

To find the name of the currently-installed lrose package, run:

```
  brew list
```

This could be ```lrose```, ```lrose-core```, ```radx``` etc.

Suppose it is ```lrose-core```.

Uninstall it as follows:

```
  brew uninstall lrose-core
```

If needed, uninstall fractl, vortrac and samurai:

```
  brew uninstall lrose-fractl lrose-vortrac lrose-samurai
```

Next, download the new version of the brew formula, from:

* [https://github.com/NCAR/lrose-core/releases](https://github.com/NCAR/lrose-core/releases)

Choose from the appropriate distribution.

Then:

```
  cd ~/Downloads
  brew install lrose-core.rb
```

See [install](#install) for checking on the install.

<a name="fractl"/>

## 6. Installing fractl (DO NOT use admin privileges)

The ```fractl``` application performs dual-Doppler analysis. It is maintained by CSU. ```fractl``` is dependent on ```lrose-core```.

fractl can be installed using the ```lrose-fractl.rb``` brew formula.


Download ```lrose-fractl.rb``` from:

* [https://github.com/NCAR/lrose-core/releases](https://github.com/NCAR/lrose-core/releases)

To perform the install:

```
  cd ~/Downloads
  brew install lrose-fractl.rb
```

Note that homebrew often prints warnings that are benign.
This is because we are using formulae rather than casks.
For example you should expect to see:

```
  Error: Failed to load cask: lrose-fractl.rb
  Cask 'lrose-fractl' is unreadable
  Warning: Treating lrose-fractl.rb as a formula.
```

To upgrade:

```
  brew uninstall lrose-fractl
  brew install lrose-fractl.rb
```

<a name="vortrac"/>

## 7. Installing vortrac (DO NOT use admin privileges)

The ```vortrac``` application analyses rotations in single Doppler data. It is maintained by CSU. ```vortrac``` is dependent on ```lrose-core```.

```vortrac``` can be installed using the ```lrose-vortrac.rb``` brew formula.

Download ```lrose-vortrac.rb``` from:

* [https://github.com/NCAR/lrose-core/releases](https://github.com/NCAR/lrose-core/releases)

To perform the install:

```
  cd ~/Downloads
  brew install lrose-vortrac.rb
```

Note that homebrew often prints warnings that are benign.
This is because we are using formulae rather than casks.
For example you should expect to see:

```
  Error: Failed to load cask: lrose-vortrac.rb
  Cask 'lrose-vortrac' is unreadable
  Warning: Treating lrose-vortrac.rb as a formula.
```

To upgrade:

```
  brew uninstall lrose-vortrac
  brew install lrose-vortrac.rb
```

<a name="samurai"/>

## 8. Installing samurai (DO NOT use admin privileges)

The ```samurai``` performs multi-Doppler retrievals. It is maintained by CSU. ```samurai``` is dependent on ```lrose-core```.

```samurai``` can be installed using the ```lrose-samurai.rb``` brew formula.

Download ```lrose-samurai.rb``` from:

* [https://github.com/NCAR/lrose-core/releases](https://github.com/NCAR/lrose-core/releases)

To perform the install:

```
  cd ~/Downloads
  brew install lrose-samurai.rb
```

Note that homebrew often prints warnings that are benign.
This is because we are using formulae rather than casks.
For example you should expect to see:

```
  Error: Failed to load cask: lrose-samurai.rb
  Cask 'lrose-samurai' is unreadable
  Warning: Treating lrose-samurai.rb as a formula.
```

To upgrade:

```
  brew uninstall lrose-samurai
  brew install lrose-samurai.rb
```


