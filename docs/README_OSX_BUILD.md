## Building LROSE for OS X

### Install Apple's compilers

Install either the XCode development environment or a stand-alone version of the
XCode command lines tools.  If you intend to do lots of Apple development and
want to use Apple's IDE, install XCode.

#### Installing complete XCode

To install the full XCode package, get an Apple ID and register for the Apple App Store.

You will need to provide a credit card, so Apple can charge you if you actually buy anything.  
However, XCode is free.

From the App Store, install XCode.
Start XCode, open the preferences window, select the 'Downloads' tab, and 
install "Command Line Tools"

#### Installing stand-alone XCode command line tools

Alternatively, you can install the stand-alone XCode command line tools by downloading
"Command Line Tools" from:

```
  http://developer.apple.com/downloads
```

You will need to register for a free Apple id, no credit card is required.

### Install homebrew

Install [homebrew](http://mxcl.github.com/homebrew/)

Homebrew allows you to install libraries needed to build Radx.

### Manual LROSE install

```
  brew install szip
  brew install hdf5 --enable-cxx
  brew install --enable-cxx-compat netcdf
  brew install udunits
  brew install fftw
  tar xvf <path-to-lrose>.src.tgz
  ./configure
  make install
```

### Automated LROSE install

```
  brew install https://github.com/NCAR/lrose-core/releases/download/lrose-yyyymmdd/lrose.rb
```

where you replace `yyyymmdd` with the relevant LROSE release date.




