Build instructions for LROSE for OS X
=====================================

1) Install Apple's compilers.

Install either the XCode development environment or a stand-alone version of the
XCode command lines tools.  If you intend to do lots of Apple development and
want to use Apple's IDE, install XCode.

1a) To install XCode, get an Apple ID and register for the Apple App Store.
(You will need to provide a credit card, so Apple can charge you if you actually buy anything.  
However, XCode is free.)

From the App Store, install XCode.
Start XCode, open the preferences window, select the 'Downloads' tab, and 
install "Command Line Tools"

-----OR-----

1b) To install the stand-alone XCode command line tools:
download "Command Line Tools" from 	http://developer.apple.com/downloads
You'll need to register for a free Apple id, no credit card is required.

-------------------------------------

2) Install "homebrew" from http://mxcl.github.com/homebrew/
Homebrew allows you to install libraries needed to build Radx.

-------------------------------------

3) Install radx manually (3a) or automatically (3b)

3a) Manual install: 

$ brew install szip
$ brew install hdf5 --enable-cxx
$ brew install --enable-cxx-compat netcdf
$ brew install udunits
$ brew install fftw

$ tar xvf <path-to-radx>.src.tgz
$ ./configure
$ make install

3b) Automated install
$ brew install ftp://ftp.rap.ucar.edu/pub/titan/lrose/lrose.rb


