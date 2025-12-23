# Homebrew LROSE install - MAC OSX

1. [prepare](#prepare)
2. [install](#install)
3. [verify](#verify)
4. [upgrade](#upgrade)
5. [fractl](#fractl)
5. [vortrac](#vortrac)
7. [samurai](#samurai)

<a name="prepare"/>

## 1. Prepare (requires admin privileges)

### Install XCode and the Command Line Tools

Install either the XCode development environment or a stand-alone version of the
XCode command line tools.  If you intend to do lots of Apple development and
want to use an IDE, then install XCode.

You also need to install Homebrew. See:

* [Install XCode and Homebrew](../build/lrose_package_dependencies.osx.md)

<a name="install"/>

## 2. Install (DO NOT use admin privileges)

**NOTE - when using brew, do NOT use admin privileges.**
**Always run brew as yourself, otherwise you will create permission problems.**

Homebrew now gets the install formula files from:

```
  https://github.com/NCAR/homebrew-lrose/tree/main/Formula
```

Run homebrew to install the package you want.

The following are your options:

```
  brew install NCAR/lrose/lrose-core
  brew install NCAR/lrose/lrose-fractl
  brew install NCAR/lrose/lrose-vortrac
  brew install NCAR/lrose/lrose-samurai
```

```lrose-core``` is a dependency for the other packages. So install it first.

While homebrew is building the package, it creates log files so you can track the progress.

The location of the log files for the various distributions will be:

```
  ~/Library/Logs/Homebrew/lrose-core
  ~/Library/Logs/Homebrew/lrose-fractl
  ~/Library/Logs/Homebrew/lrose-vortrac
  ~/Library/Logs/Homebrew/lrose-samurai
```

You will see the following log files:

```
  01.make.log
  01.make.cc.log
  02.make.log
  02.make.cc.log
```

The log files contain the following:

* 01 logs: preparation
* 02.make.log: performing the build
* 02.make.cc.log: verbose version of 02.make

You can watch the progress using:

```
  tail -f 02.make
```

If the build is successful, lrose will be installed in:

```
  /opt/homebrew/include
  /opt/homebrew/lib
  /opt/homebrew/bin
```

In these directories, links will be created that point to the actual files
in ```/opt/homebrew/Cellar```.

On older Intel-based systems, the installation directories are:

```
  /usr/local/include
  /usr/local/lib
  /usr/local/bin
```

<a name="verify"/>

## 3. Verify the installation

Try the commands:
```
  /opt/homebrew/bin/RadxPrint -h
  /opt/homebrew/bin/RadxConvert -h
  /opt/homebrew/bin/Radx2Grid -h
  /opt/homebrew/bin/HawkEye
```

<a name="upgrade"/>

## 4. Upgrade to a new LROSE version (DO NOT use admin privileges)

When the time comes to upgrade, you will first need to uninstall the current version.

To find the name of the currently-installed lrose package, run:

```
  brew list
```

You should see ```lrose-core```.

Uninstall as follows:

```
  brew uninstall lrose-fractl
  brew uninstall lrose-vortrac
  brew uninstall lrose-samurai
  brew uninstall lrose-core
```

Then run:

```
  brew install NCAR/lrose/lrose-core
  brew install NCAR/lrose/lrose-fractl
  brew install NCAR/lrose/lrose-vortrac
  brew install NCAR/lrose/lrose-samurai
```

See [install](#install) for checking on the install.

<a name="fractl"/>

## 5. Installing fractl (DO NOT use admin privileges)

The ```fractl``` application performs dual-Doppler analysis. It is maintained by CSU.

```fractl``` is dependent on ```lrose-core```.

First install ```lrose-core```.

To install fractl, run:

```
  brew install NCAR/lrose/lrose-fractl
```

fractl will be installed as:

```
  /opt/homebrew/bin/fractl
```

To upgrade:

```
  brew uninstall lrose-fractl
  brew install NCAR/lrose/lrose-fractl
```

<a name="vortrac"/>

## 6. Installing vortrac (DO NOT use admin privileges)

The ```vortrac``` application analyses rotations in single Doppler data. It is maintained by CSU.

```vortrac``` is dependent on ```lrose-core```.

```vortrac``` can be installed using the ```lrose-vortrac.rb``` brew formula.

First install ```lrose-core```.

To install vortrac, run:

```
  brew install NCAR/lrose/lrose-vortrac
```

vortrac will be installed as:

```
  /opt/homebrew/bin/vortrac
```

To upgrade:

```
  brew uninstall lrose-vortrac
  brew install NCAR/lrose/lrose-vortrac
```

<a name="samurai"/>

## 7. Installing samurai (DO NOT use admin privileges)

The ```samurai``` application performs multi-Doppler retrievals. It is maintained by CSU.

```samurai``` is dependent on ```lrose-core```.

First install ```lrose-core```.

To install samurai, run:

```
  brew install NCAR/lrose/lrose-samurai
```

samurai will be installed as:

```
  /opt/homebrew/bin/samurai
```

To upgrade:

```
  brew uninstall lrose-samurai
  brew install NCAR/lrose/lrose-samurai
```



