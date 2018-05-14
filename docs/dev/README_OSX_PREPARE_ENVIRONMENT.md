## PREPARING the OSX Environment for LROSE
This step is necessary when using the binary or source release.

---------

### Install the dependencies:
```
brew install netcdf
```

This is a typical result ...
```
==> Installing dependencies for netcdf: gcc, szip, hdf5
==> Installing netcdf dependency: gcc
Warning: Building gcc from source:
  The bottle needs the Xcode CLT to be installed.
Error: /usr/local/opt/gmp not present or broken
Please reinstall gmp. Sorry :(
```

The next step is ...
```
xcode-select --install
```

A typical result ...
```
xcode-select: note: install requested for command line developer tools
```

These steps were suggested here:

https://www.google.com/search?q=xcode-select+install&oq=xcode-&aqs=chrome.3.69i57j0j69i60j0l3.7918j0j7&sourceid=chrome&ie=UTF-8

Then check dynamic library dependencies ...
```
otool -L appname
```

And make sure that all the libraries listed are present on your machine.
