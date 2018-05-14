## Building LROSE using AUTOMAKE and CONFIGURE

This documents how to check out and build an LROSE release
using automake and configure.

This uses the script:

```
  checkout_and_build_auto.py
```

### 1. Choose your install directory (prefix)

The default install location is: `${HOME}/lrose`

### 2. Check out `lrose-core`

```
  mkdir ~/git
  cd ~/git
  git clone https://github.com/NCAR/lrose-core
```

### 3. Check the usage

```
  cd ~/git/lrose-core
  ./build/checkout_and_build_auto.py --help

  Usage: checkout_and_build_auto.py [options]
  Options:
    -h, --help           show this help message and exit
    --clean              Cleanup tmp build dir
    --debug              Set debugging on
    --verbose            Set verbose debugging on
    --package=PACKAGE    Package name. Options are: lrose (default), cidd, radx,
                         titan, lrose-blaze
    --prefix=PREFIX      Install directory
    --buildDir=BUILDDIR  Temporary build dir
    --static             use static linking, default is dynamic
    --scripts            Install scripts as well as binaries
```

`package` defaults to `lrose`

`prefix` defaults to `${HOME}/lrose`

Available packages are:

```
  lrose lrose-blaze radx titan cidd
```

### 4. Run the checkout and auto build

The following example installs the lrose-blaze release in `~/lrose`:

```
  cd ~/git/lrose-core
  ./build/checkout_and_build_auto.py --debug --prefix ~/lrose --package lrose-blaze
```

Another example - build lrose and install in `/usr/local/lrose`:

```
  cd ~/git/lrose-core
  ./build/checkout_and_build_auto.py --debug --prefix /usr/local/lrose --package lrose
```

