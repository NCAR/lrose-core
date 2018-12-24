# Homebrew LROSE install - MAC OSX

1. [prepare](#prepare)
2. [download](#download)
3. [install](#install)
4. [verify](#verify)

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

### Installing stand-alone XCode command line tools

Alternatively, you can install the stand-alone XCode command line tools by downloading
"Command Line Tools" from:

```
  http://developer.apple.com/downloads
```

You will need to register for a free Apple id, no credit card is required.

### Install homebrew

The default location for homebrew is /usr/local. So you need write permission
to /usr/local to perform the install.

Run the following ruby script:

```
  /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  /usr/local/bin/brew update
```

### Install required packages, using brew

```
  brew install pkg-config
  brew install szip
  brew install hdf5 --enable-cxx
  brew install netcdf
  brew install udunits
  brew install fftw
  brew install flex
  brew install jasper
  brew install jpeg
  brew install libpng
  brew install qt
```
<a name="download"/>

## 2. Download

You need to download the brew formula from the lrose repository.
This formula is used to perform the homebrew build.

Download lrose.rb from:

```
  https://github.com/NCAR/lrose-core/releases 
```

Choose from the available distributions.

<a name="install"/>

## 3. Install

```
  cd ~/Downloads
  brew install lrose.rb
```

This will install lrose in:

```
  /usr/local/opt/lrose/include
  /usr/local/opt/lrose/lib
  /usr/local/opt/lrose/bin
```

<a name="verify"/>

## 4. Verify the installation

Try the commands:
```
  /usr/local/opt/lrose/bin/RadxPrint -h
  /usr/local/opt/lrose/bin/RadxConvert -h
  /usr/local/opt/lrose/bin/Radx2Grid -h
  /usr/local/opt/lrose/bin/HawkEye
```


