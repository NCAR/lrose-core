# NOTES on installing homebrew - MAC OSX

## 1. Install brew

To do this you need admin privileges:

```
  /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

## 2. Ensure the /usr/local directories are owned by you

```
sudo chown -R $(whoami) /usr/local/Cellar /usr/local/Homebrew /usr/local/bin /usr/local/etc /usr/local/etc/bash_completion.d /usr/local/include /usr/local/lib /usr/local/lib/pkgconfig /usr/local/opt /usr/local/sbin /usr/local/share /usr/local/share/aclocal /usr/local/share/doc /usr/local/share/info /usr/local/share/locale /usr/local/share/man /usr/local/share/man/man1 /usr/local/share/man/man3 /usr/local/share/man/man5 /usr/local/share/man/man7 /usr/local/share/zsh /usr/local/share/zsh/site-functions /usr/local/var/homebrew/linked /usr/local/var/homebrew/locks
```

## 3. Download lrose.rb

Go to lrose-core on github:

```
  https://github.com/NCAR/lrose-core/releases 
```

Download `lrose.rb`

Then:

```
  cd Downloads
  brew install lrose.rb
```

## 4. Terminal output while building lrose-core

You should see output similar to the following:

```

==> Installing dependencies for lrose: gmp, isl, mpfr, libmpc, gcc, szip, hdf5, netcdf, udunits, fftw, gettext, flex, jpeg, jasper, libpng, qt and pkg-config

==> Installing lrose dependency: gmp
==> Downloading https://homebrew.bintray.com/bottles/gmp-6.1.2_2.high_sierra.bottle.tar.gz
==> Pouring gmp-6.1.2_2.high_sierra.bottle.tar.gz
    /usr/local/Cellar/gmp/6.1.2_2: 18 files, 3.1MB

==> Installing lrose dependency: isl
==> Downloading https://homebrew.bintray.com/bottles/isl-0.20.high_sierra.bottle.tar.gz
==> Pouring isl-0.20.high_sierra.bottle.tar.gz
    /usr/local/Cellar/isl/0.20: 71 files, 3.9MB

==> Installing lrose dependency: mpfr
==> Downloading https://homebrew.bintray.com/bottles/mpfr-4.0.1.high_sierra.bottle.tar.gz
==> Pouring mpfr-4.0.1.high_sierra.bottle.tar.gz
    /usr/local/Cellar/mpfr/4.0.1: 28 files, 4.6MB

==> Installing lrose dependency: libmpc
==> Downloading https://homebrew.bintray.com/bottles/libmpc-1.1.0.high_sierra.bottle.tar.gz
==> Pouring libmpc-1.1.0.high_sierra.bottle.tar.gz
    /usr/local/Cellar/libmpc/1.1.0: 12 files, 353.8KB

==> Installing lrose dependency: gcc
==> Downloading https://homebrew.bintray.com/bottles/gcc-8.2.0.high_sierra.bottle.1.tar.gz
==> Pouring gcc-8.2.0.high_sierra.bottle.1.tar.gz
    /usr/local/Cellar/gcc/8.2.0: 1,495 files, 344.8MB

==> Installing lrose dependency: szip
==> Downloading https://homebrew.bintray.com/bottles/szip-2.1.1_1.high_sierra.bottle.tar.gz
==> Pouring szip-2.1.1_1.high_sierra.bottle.tar.gz
    /usr/local/Cellar/szip/2.1.1_1: 11 files, 108.7KB

==> Installing lrose dependency: hdf5
==> Downloading https://homebrew.bintray.com/bottles/hdf5-1.10.4.high_sierra.bottle.tar.gz
==> Pouring hdf5-1.10.4.high_sierra.bottle.tar.gz
    Warning: hdf5 dependency gcc was built with a different C++ standard
    library (libstdc++ from clang). This may cause problems at runtime.
    /usr/local/Cellar/hdf5/1.10.4: 262 files, 14.8MB

==> Installing lrose dependency: netcdf
==> Downloading https://homebrew.bintray.com/bottles/netcdf-4.6.2.high_sierra.bottle.tar.gz
==> Pouring netcdf-4.6.2.high_sierra.bottle.tar.gz
    Warning: netcdf dependency gcc was built with a different C++ standard
    library (libstdc++ from clang). This may cause problems at runtime.
    /usr/local/Cellar/netcdf/4.6.2: 85 files, 6.2MB

==> Installing lrose dependency: udunits
==> Downloading https://homebrew.bintray.com/bottles/udunits-2.2.26.high_sierra.bottle.tar.gz
==> Pouring udunits-2.2.26.high_sierra.bottle.tar.gz
    /usr/local/Cellar/udunits/2.2.26: 29 files, 538.7KB

==> Installing lrose dependency: fftw
==> Downloading https://homebrew.bintray.com/bottles/fftw-3.3.8.high_sierra.bottle.tar.gz
==> Pouring fftw-3.3.8.high_sierra.bottle.tar.gz
   /usr/local/Cellar/fftw/3.3.8: 52 files, 10.8MB

==> Installing lrose dependency: gettext
==> Downloading https://homebrew.bintray.com/bottles/gettext-0.19.8.1.high_sierra.bottle.tar.g
==> Pouring gettext-0.19.8.1.high_sierra.bottle.tar.gz
==> Caveats
    gettext is keg-only, which means it was not symlinked into /usr/local,
    because macOS provides the BSD gettext library & some software gets confused
    if both are in the library path.
    If you need to have gettext first in your PATH run:
      echo 'setenv PATH /usr/local/opt/gettext/bin:$PATH' >> ~/.cshrc
    For compilers to find gettext you may need to set:
      setenv LDFLAGS -L/usr/local/opt/gettext/lib;
      setenv CPPFLAGS -I/usr/local/opt/gettext/include;
==> Summary
   /usr/local/Cellar/gettext/0.19.8.1: 1,935 files, 16.9MB

==> Installing lrose dependency: flex
==> Downloading https://homebrew.bintray.com/bottles/flex-2.6.4.high_sierra.bottle.tar.gz
==> Pouring flex-2.6.4.high_sierra.bottle.tar.gz
==> Caveats
    flex is keg-only, which means it was not symlinked into /usr/local,
    because some formulae require a newer version of flex.
    If you need to have flex first in your PATH run:
      echo 'setenv PATH /usr/local/opt/flex/bin:$PATH' >> ~/.cshrc
    For compilers to find flex you may need to set:
      setenv LDFLAGS -L/usr/local/opt/flex/lib;
      setenv CPPFLAGS -I/usr/local/opt/flex/include;
==> Summary
   /usr/local/Cellar/flex/2.6.4: 45 files, 1.4MB

==> Installing lrose dependency: jpeg
==> Downloading https://homebrew.bintray.com/bottles/jpeg-9c.high_sierra.bottle.tar.gz
==> Pouring jpeg-9c.high_sierra.bottle.tar.gz
   /usr/local/Cellar/jpeg/9c: 21 files, 724.5KB

==> Installing lrose dependency: jasper
==> Downloading https://homebrew.bintray.com/bottles/jasper-2.0.14.high_sierra.bottle.tar.gz
==> Pouring jasper-2.0.14.high_sierra.bottle.tar.gz
   /usr/local/Cellar/jasper/2.0.14: 39 files, 993.2KB

==> Installing lrose dependency: libpng
==> Downloading https://homebrew.bintray.com/bottles/libpng-1.6.36.high_sierra.bottle.tar.gz
==> Pouring libpng-1.6.36.high_sierra.bottle.tar.gz
    /usr/local/Cellar/libpng/1.6.36: 27 files, 1.2MB

==> Installing lrose dependency: qt
==> Downloading https://homebrew.bintray.com/bottles/qt-5.12.0.high_sierra.bottle.tar.gz
==> Pouring qt-5.12.0.high_sierra.bottle.tar.gz
==> Caveats
    We agreed to the Qt open source license for you.
    If this is unacceptable you should uninstall.
    qt is keg-only, which means it was not symlinked into /usr/local,
    because Qt 5 has CMake issues when linked.
    If you need to have qt first in your PATH run:
      echo 'setenv PATH /usr/local/opt/qt/bin:$PATH' >> ~/.cshrc
    For compilers to find qt you may need to set:
      setenv LDFLAGS -L/usr/local/opt/qt/lib;
      setenv CPPFLAGS -I/usr/local/opt/qt/include;
==> Summary
   /usr/local/Cellar/qt/5.12.0: 9,689 files, 318.9MB

==> Installing lrose dependency: pkg-config
==> Downloading https://homebrew.bintray.com/bottles/pkg-config-0.29.2.high_sierra.bottle.tar.
==> Pouring pkg-config-0.29.2.high_sierra.bottle.tar.gz
   /usr/local/Cellar/pkg-config/0.29.2: 11 files, 627.2KB

==> Installing lrose
==> Downloading https://github.com/NCAR/lrose-core/releases/download/lrose-20181223/lrose-2018
==> Downloading from https://github-production-release-asset-2e65be.s3.amazonaws.com/51408988/
==> ./configure --prefix=/usr/local/Cellar/lrose/20181223
==> make install
==> Installing lrose
==> Downloading https://github.com/NCAR/lrose-core/releases/download/lrose-20181223/lrose-2018
==> Downloading from https://github-production-release-asset-2e65be.s3.amazonaws.com/51408988/
==> ./configure --prefix=/usr/local/Cellar/lrose/20181223
==> make install
==> rsync -av share /usr/local/Cellar/lrose/20181223
    Warning: lrose dependency gcc was built with a different C++ standard
    library (libstdc++ from clang). This may cause problems at runtime.
   /usr/local/Cellar/lrose/20181223: 2,402 files, 102.5MB, built in 63 minutes 33 seconds

==> Caveats
    For pkg-config to find qt you may need to set:
    setenv PKG_CONFIG_PATH /usr/local/opt/qt/lib/pkgconfig;

```
