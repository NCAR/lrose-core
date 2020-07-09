# Homebrew LROSE install - MAC OSX

1. [prepare](#prepare)
2. [download](#download)
3. [install](#install)
4. [verify](#verify)
5. [upgrade](#upgrade)

<a name="prepare"/>

## 1. Prepare

Install either the XCode development environment or a stand-alone version of the
XCode command line tools.  If you intend to do lots of Apple development and
want to use an IDE, then install XCode.

### Installing complete XCode

To install the full XCode package, get an Apple ID and register for the Apple App Store.

You will need to provide a credit card, so Apple can charge you if you actually buy anything.  
However, XCode is free.

From the App Store, install XCode.
Start XCode, open the preferences window, select the 'Downloads' tab, and 
install "Command Line Tools"

You may also need to run:

```
  xcode-select --install
```

### Installing XCode command line tools

Install the stand-alone XCode command line tools by downloading "Command Line Tools" from:

```
  http://developer.apple.com/downloads
```

You will need to register for a free Apple id, no credit card is required.

### Reboot after XCode install

You should reboot after installing or upgrading XCode.

The reboot will perform some steps to complete the XCode install.

### Install homebrew

The default location for homebrew is /usr/local. So you need write permission
to /usr/local to perform the install.

Run the following ruby script:

```
  /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  /usr/local/bin/brew update
```

## 2. Download

You need to download the brew formula from the lrose repository.
This formula is used to perform the homebrew build.

Download ```lrose-core.rb``` from:

```
  https://github.com/NCAR/lrose-core/releases 
```

Choose the latest version.

<a name="install"/>

## 3. Install

Let us assume you have downloaded the latest core distribution, containing all of the core apps.

```
  cd ~/Downloads
  brew install lrose-core.rb
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
```

You can watch the progress using:

```
  tail -f 01.configure
  tail -f 02.make
```

If the build is successful, lrose will be installed in:

```
  /usr/local/opt/lrose/include
  /usr/local/opt/lrose/lib
  /usr/local/opt/lrose/bin
```

In these directories, links will be created that point to the actual files
in ```/usr/local/Cellar```.

See also: [Homebrew Notes](./homebrew_notes.md)

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

## 5. Upgrade to a new version

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
Next, download the new version of the brew formula, from:

```
  https://github.com/NCAR/lrose-core/releases 
```

Choose from the appropriate distribution.

Then:

```
  cd ~/Downloads
  brew install lrose-core.rb
```

See [install](#install) for checking on the install.


