
## Installing an LROSE Source Release (Mac)

[prepare](#prepare)
[install](#install)
[verify](#verify)


<a name="prepare"/>

### Install Apple's compilers

Install either the XCode development environment or a stand-alone version of the
XCode command line tools.  If you intend to do lots of Apple development and
want to use Apple's IDE, then install XCode.

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

### Install required packages, using brew

```
  brew install szip
  brew install hdf5 --enable-cxx
  brew install --enable-cxx-compat netcdf
  brew install udunits
  brew install fftw
  brew install flex
  brew install jasper
  brew install jpeg
  brew install qt
  tar xvf <path-to-lrose>.src.tgz
  ./configure
  make install
```
<a name="install"/>
### Prepare build directory

Create a directory for the distribution:

```
  cd
  mkdir lrose_build
  cd lrose_build
```

### Download source release for OSX

Download the source tar file from:

```
  https://github.com/NCAR/lrose-core/releases
```

A typical source release would be:

```
  lrose-20160823.src.osx.tgz
```

### Untar the distribution

```
  cd lrose_build
  tar xvfz lrose-20160823.src.osx.tgz
```

The distribution will be unpacked into a subdirectory:

```
  lrose_build/lrose-20160823.src.osx
```

### Run the build scripts:

```
  cd lrose_build/lrose-20160823.src
  ./build_lrose.py --prefix installDir
```

The default prefix is $HOME/lrose.

This will install in:

```
  installDir/include
  installDir/lib
  installDir/bin
```

### Checking the build

The build checks are run automatically at the end of the build script.

However, you also can run the checks independently:

After the build, you can check the build as follows:

```
  ./build/check_libs -x installDir
  ./build/check_apps -x installDir
```

### Handling build errors

If the build does not complete successfully, you will need to
track down the errors. It is the first errors in the build that
are the most important.

If you get errors, go into the directory giving problems, and
run the make as follows:

```
  make |& less
```

and scroll for errors.

Alternatively, run

```
  make >& make.log
```

and then inspect the make.log file.

## Troubleshooting

[Make sure your environment is ready for installation.](./README_OSX_PREPARE_ENVIRONMENT.md)

