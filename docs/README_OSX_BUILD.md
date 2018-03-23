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

The default location for homebrew is /usr/local. So you need write permission
to /usr/local to perform the install.

Run the following ruby script:

```
  /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
  /usr/local/bin/brew update
```

### Optional homebrew install in home directory

If access is not available to /usr/local, you can install in your home directory instead.

```
  cd
  git clone https://github.com/mxcl/homebrew.git
```

Add brew to your path, based on the login shell you use.
Generally you will do this by editing your shell startup scripts.

If you use bash, add the following to .bashrc:
```
  export PATH=${HOME}/homebrew/bin:${PATH}
```

If you use tcsh, add the following to .cshrc:
```
  set path = ( ${HOME}/homebrew/bin ${path} )
  source ~/.cshrc
```

Then run:

```
  brew update
```

### Install required libraries

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

### Automated LROSE install (Not yet available)

```
  brew install https://github.com/NCAR/lrose-core/releases/download/lrose-yyyymmdd/lrose.rb
```

where you replace `yyyymmdd` with the relevant LROSE release date.




