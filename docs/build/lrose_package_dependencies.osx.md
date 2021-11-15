# LROSE-CORE package dependencies - OSX

## Install XCode

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

* [http://developer.apple.com/downloads](http://developer.apple.com/downloads)

You will need to register for a free Apple id, no credit card is required.

### Reboot after XCode install

You should reboot after installing or upgrading XCode.

The reboot will perform some steps to complete the XCode install.

## Install homebrew

Run the following:

```
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  /usr/local/bin/brew update
```

The default location for homebrew is ```/usr/local```. So your account needs write permission
to ```/usr/local``` to perform the install.

More specifically, you need write permission for the following directories:

```
  /usr/local/Caskroom
  /usr/local/Cellar
  /usr/local/Frameworks
  /usr/local/Homebrew
  /usr/local/bin
  /usr/local/etc
  /usr/local/include
  /usr/local/lib
  /usr/local/opt
  /usr/local/sbin
  /usr/local/share
  /usr/local/var

```

WARNING - when using brew to install packages, do not use ```root``` or ```sudo```. This is very important. If you do, it will lead to permissions problems.

## Install required packages using brew

Use Homebrew to install the required packages:

```
  brew install hdf5 netcdf fftw flex jpeg libpng libzip qt szip pkg-config cmake rsync libx11 libxext
```

